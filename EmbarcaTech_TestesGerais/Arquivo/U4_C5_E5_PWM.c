/*  EMBARCATECH - 2024/25
    UNIDADE 4   CAP. 5
    ALUNO: CARLOS FERNANDO MATTOS DO AMARAL
    DESAFIO: 5 
    Modifique o exemplo de código apresentado na videoaula (reproduzido abaixo) para controlar os LEDs RGB da placa BitDogLab 
    usando o módulo PWM e interrupções, seguindo as orientações a seguir:
    (1) O LED vermelho deve ser acionado com um PWM de 1kHz.
    (2) O LED verde deve ser acionado com um PWM de 10kHz.
    (3) O Duty Cycle deve ser iniciado em 5% e atualizado a cada 2 segundos em incrementos de 5%. Quando atingir o valor máximo,
        deve retornar a 5% e continuar a incrementando. - Fazer isso para ambos os LEDs: verde e vermelho.
    (4) Tente controlar frequência e o Duty Cycle do LED azul de forma independente do que fez nos LEDs vermelho e verde. Você 
        consegue? Por que não?  
*/

#include <stdbool.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define red 13
#define blue 12
#define green 11
#define buttonA 5 // Botão A
#define buttonB 6 // Botão B

// Parametros de Configuração (enunciado):
#define PERIOD 2000   // Período do PWM
#define DIVIDER_RED_BLUE 62.5    // Divisor fracional do Clock para PWM de 1kHz (=125000k/(62.5*2k))
#define DIVIDER_GREEN 6.25    // Divisor fracional do Clock para PWM de 10kHz (=125000k/(6.25*2k))
// A frequencia do Azul (gpio 12) é a mesma do Vermelho (gpio 13), poeque eles dividem o mesmo canal (6)
// mas podemos alterar o duty cycle individualmente (faremos abaixo)
#define STEP_TIME_MS 50    // Tempo entre uma alteração e outra de luminosidade 
uint16_t led_level[] = {100, 100, 100}; // vamos utilizar levels diferentes para termos intensidades individuais das cores
uint16_t level_increment[] = {100, 100, 100};
bool individual = false; // defino se examino individualmente cada cor ou em conjutno
bool btn[] = {false, false}; // {estado atual, estado anterior}  false = solto, true = pressionado

void setup_pwm() {
    uint slice;
    // Vermelho
    gpio_set_function(red, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(red);
    printf("Canal do Vermelho: %d\n", slice);
    pwm_set_clkdiv(slice, DIVIDER_RED_BLUE);
    pwm_set_wrap(slice, PERIOD);
    pwm_set_gpio_level(red, led_level[0]);
    pwm_set_enabled(slice, true);
    // Azul
    gpio_set_function(blue, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(blue);
    printf("Canal do Azul: %d\n", slice);
    pwm_set_clkdiv(slice, DIVIDER_RED_BLUE);
    pwm_set_wrap(slice, PERIOD);
    pwm_set_gpio_level(blue, led_level[1]);
    pwm_set_enabled(slice, true);
    // Green
    gpio_set_function(green, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(green);
    printf("Canal do Verde: %d\n", slice);
    pwm_set_clkdiv(slice, DIVIDER_GREEN);
    pwm_set_wrap(slice, PERIOD);
    pwm_set_gpio_level(green, led_level[2]);
    pwm_set_enabled(slice, true);
}

/**
 * @brief Função Principal
 */
int main() {
    uint8_t mode[] = {0, 0}; // {A, B} modo A refere-se a branco ou colorido, modo B refere-se a qual cor mostrar
    bool btnA[] = {1, 1};
    bool btnB[] = {1, 1};
    stdio_init_all(); // permite a utilização do monitor serial

    sleep_ms(5000); // tempo para ligar o monitor

    // Iniciando PWM e Botões
    setup_pwm();
    gpio_init(buttonA);
    gpio_set_dir(buttonA, GPIO_IN);
    gpio_pull_up(buttonA);
    gpio_init(buttonB);
    gpio_set_dir(buttonB, GPIO_IN);
    gpio_pull_up(buttonB);
    
    printf("Iniciou o programa.\n");

    while(true) {
        btnA[0] = gpio_get(buttonA);
        btnB[0] = gpio_get(buttonB);

        // Avaliando esdado do botão:
        if (btnA[0] != btnA[1]) {  // mudou o estado
            btnA[1] = btnA[0];  // impede de entrar aqui até trocar estado novamente
            if (btnA[0] == true) {
                // btn A foi pressionado e não liberado
                if (mode[0] == 0) {  
                    mode[0] = 1;  // colorido, a intensidade das cores estão defasadas
                    led_level[0] = 100;
                    led_level[1] = 700;
                    led_level[2] = 1300; 
                } else {
                    mode[0] = 0;  // Branco - todos os leds sincronizados
                    led_level[0] = PERIOD * 0.05;  
                    led_level[1] = PERIOD * 0.05;
                    led_level[2] = PERIOD * 0.05; 
                }
                printf("Modo %s\n", mode[0]==0?"Branco":"Colorido");
            }
        }
        if (btnB[0] != btnB[1]) {  // mudou o estado
            btnB[1] = btnB[0];  // impede de entrar aqui até trocar estado novamente
            if (btnB[0] == true) {
                // btn B foi pressionado e não liberado
                if (++mode[1] > 3) mode[1] = 0;  // incrementa o modo e reseta se for 4 (não tem 5 modos)
                printf("Modo de amostra de cor: %d \n", mode[1]);
            }
        }

        // EXPLORANDO OS MODOS: mode[0] = A, mode[1] = B
        //      A                   B
        //   0 = Branco         0 = todas as cores juntas
        //   1 = Colorido       1 = apenas vermelho
        //                      2 = apenas verde
        //                      3 = apenas azul

        for (int i = 0; i < 3; i++) {
            led_level[i] += level_increment[i];
            if (led_level[i] >= PERIOD) led_level[i] = (PERIOD * 0.05);
        }

        int16_t level_show[] = {led_level[0], led_level[1], led_level[2]}; // assim não altera o led_level, e a contagem continua rolando
        
        if (mode[1] == 1) {  // se for para mostrar só a vermelha, apaga o resto
            level_show[1] = 0;
            level_show[2] = 0;
        } else if (mode[1] == 2) { // se for para mostrar só a azul, apaga o resto
            level_show[0] = 0;
            level_show[2] = 0;
        } else if (mode[1] == 3) { // se for para mostrar só a verde, apaga o resto
            level_show[0] = 0;
            level_show[1] = 0;
        }
        //printf("level_show(%d, %d, %d)\n", level_show[0], level_show[1], level_show[2]);
        pwm_set_gpio_level(red, level_show[0]);
        pwm_set_gpio_level(blue, level_show[1]);
        pwm_set_gpio_level(green, level_show[2]);
        sleep_ms(STEP_TIME_MS);
    }

    return 0;
}
