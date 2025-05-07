/*  EMBARCATECH - 2024/25
    UNIDADE 4   CAP. 7
    ALUNO: CARLOS FERNANDO MATTOS DO AMARAL
    CÓDIGO: 01 
    Como usar o módulo PWM do RP2040 para controlar o brilho do LED da placa BitDogLab.
*/

#include <stdbool.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"

#define red 13
#define blue 12
#define green 11
#define btn_pin 5  // Botão A

// Parametros de Configuração (enunciado):
#define PERIOD_PWM 3000     // Período do PWM (ms) 2000 = wrap
#define DIVIDER_PWM 16.0    // Divisor fracional do Clock para PWM
#define STEP_TIME_MS 10    // Tempo entre uma alteração e outra de luminosidade 
int STEP_WHITE[] = {10, 10, 10};        // Passo de incremento/decremento do duty cycle
int STEP_COLORFULL[] = {3, 6, 10};    // Passo de incremento/decremento do duty cycle

// funções e variáveis
volatile bool botao[] = {false,false};
volatile bool white[] = {true, true};
volatile bool up[] = {true, true, false};
volatile int level[] = {100, 100, 100};   // nível do duty cycle para vermelho, azul, verde
volatile int *level_step = STEP_WHITE;
struct repeating_timer timer_blink;
struct repeating_timer timer_btn;

void setup_pwm() {
    uint slice;
    // Vermelho
    gpio_set_function(red, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(red);
    pwm_set_clkdiv(slice, DIVIDER_PWM);
    pwm_set_wrap(slice, PERIOD_PWM);
    pwm_set_gpio_level(red, level[0]);
    pwm_set_enabled(slice, true);
    // Azul
    gpio_set_function(blue, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(blue);
    pwm_set_clkdiv(slice, DIVIDER_PWM);
    pwm_set_wrap(slice, PERIOD_PWM);
    pwm_set_gpio_level(blue, level[1]);
    pwm_set_enabled(slice, true);
    // Green
    gpio_set_function(green, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(green);
    pwm_set_clkdiv(slice, DIVIDER_PWM);
    pwm_set_wrap(slice, PERIOD_PWM);
    pwm_set_gpio_level(green, level[2]);
    pwm_set_enabled(slice, true);
}

/**
 * @brief Leitura do Botão 
 * Avalia estatus do botão a cada 50ms, funcionando como um debouncing,
 * conta a quantidade de clicks, e aciona o pisca após o número de vezes
 * definido em n_de_clicks. Além disso aciona o timer para desligar o 
 * pisca após o tempo definido em turn_off_time_ms. 
 * @param[in] *repeating_timer ponteiro do timer de repetição
 */
bool repeating_callback(struct repeating_timer *t) {
    //printf("Botao (%b, %b)\n", botao[0], botao[1]);
    botao[0] = gpio_get(btn_pin); 
    if (botao[0] != botao[1]) { // mudou o estado do botao (só entra se o estado atual for diferente do estado anterior)
        botao[1] = botao[0]; // define o estado anterior igual ao estado atual (não entra mais neste if)
        if (botao[0] == true) { // so troca se estiver apertando e não soltando.
            white[0] = !white[0]; // troca o modo
        }
    } 
    return true;
}

bool freq_callback(struct repeating_timer *t) {
    // printf("Aqui deveria entrar na frequencia de STEP_TIME_MS\n");
    if (white[0] != white[1]) {  // Só entra se mudou de estado
        printf("white (%d)", white[0]);
        white[1] = white[0];
        if (white[0] == true) { 
            level_step = STEP_WHITE; 
            level[0] = 100; up[0] = true;
            level[1] = 100; up[1] = true;
            level[2] = 100; up[2] = true;
        } else {
            level_step = STEP_COLORFULL;
        }
    }
    for (int i = 0; i < 3; i++) {
        if (up[i] == true) {
            level[i] += level_step[i];
            if (level[i] >= PERIOD_PWM/2) up[i] = false;
        } else {
            level[i] -= level_step[i];
            if (level[i] <= 0) up[i] = true;
        }
    }
    pwm_set_gpio_level(red, level[0]);
    pwm_set_gpio_level(blue, level[1]);
    pwm_set_gpio_level(green, level[2]);
    return true;
}

/**
 * @brief Função Principal
 */
int main() {
    stdio_init_all(); // permite a utilização do monitor serial

    gpio_init(btn_pin);
    gpio_set_dir(btn_pin, GPIO_IN);
    gpio_pull_up(btn_pin);
    
    // Iniciando PWM
    setup_pwm();
  
    printf("Iniciou o programa.\n");

    add_repeating_timer_ms(50, repeating_callback, NULL, &timer_btn); // Aqui define o Timer de verificaçao do botão
    add_repeating_timer_ms(STEP_TIME_MS, freq_callback, NULL, &timer_blink); // Aqui define a freq de atualização da intensidade luminosa

    while(1) {
        tight_loop_contents();
    }

    return 0;
}
