/*  EMBARCATECH - 2024/25
    UNIDADE 4   CAP. 5
    ALUNO: CARLOS FERNANDO MATTOS DO AMARAL
    DESAFIO 1: 
    Elabore um programa para acionar um LED quando o botão A for pressionado 5 vezes, 
    tomando cuidado para registrar essa contagem. Quando o valor da contagem chegar a 5, 
    um LED deve piscar por 10 segundos, na frequência de 10 Hz.
*/

#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdbool.h>
#include <stdio.h>
#define red 13
#define blue 12
#define green 11
#define btn_pin 5  // Botão A

// Parametros de Configuração (enunciado):
#define turn_off_time_ms 10000
#define n_de_clicks 5
#define freq_piscagem 10

// funções e variáveis
void ligaPisca(bool liga);  // Liga/desliga todas as cores tornando o led Branco
void led_inverter();
void liga_led(bool liga);
volatile bool pisca_ligado = false; // registra o estado do led
volatile bool led_ligado = false;
volatile bool botao_apertado = 1;
volatile int btn_count = 0;  // a contagem do botão é feito dentro de um callback, por isso 'volatile'
volatile bool botao_solto = true;
struct repeating_timer timer_blink;
struct repeating_timer timer_btn;

/**
 * @brief Piscador do Led 
 * Esta função de CallBack é acionada repetidamente na frequencia
 * definida por freq_piscagem
 * @param[in] *repeating_timer ponteiro do repetidor.
 * @param[out] booleano indica se a função manterá a repetição
 */
bool repeating_callback_pisca_led(struct repeating_timer *t) {
    led_inverter(); // inverte o led 
    return true;
}

/**
 * @brief Temporizador para desligar Led 
 */
int64_t turn_off_callback(alarm_id_t id, void *user_data) {
   ligaPisca(false);
   return 0;
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
    botao_apertado = gpio_get(btn_pin); 
    if (botao_apertado == 0 && pisca_ligado == false && botao_solto == true) { // só entra se o botão está apertado E o led desligado
        botao_solto = false; // apertou o botão mas não soltou ainda. Não é para entrar novamente neste if.
        printf("Botão pressionado %d vez%s.\n", btn_count + 1, (btn_count+1==1?"":"es"));
        if (++btn_count == n_de_clicks) {  // incrementa o count e compara com o numero desejado
            btn_count = 0; // reseta a contagem
            ligaPisca(true);    // liga o led, de forma que não entra mais nesse if até ser desligado
            add_alarm_in_ms(turn_off_time_ms, turn_off_callback, NULL, false); // Cria o alarme para desligar o led
        }
    } else if (botao_apertado == 1) botao_solto = true; // Se o botão não está apertado, é porque ele está solto. 
    return true;
}

/**
 * @brief Função Principal
 */
int main() {
    stdio_init_all(); // permite a utilização do monitor serial
    
    // Iniciando GPIOs
    gpio_init(red);
    gpio_set_dir(red, GPIO_OUT);
    gpio_init(blue);
    gpio_set_dir(blue, GPIO_OUT);
    gpio_init(green);
    gpio_set_dir(green, GPIO_OUT);
    gpio_init(btn_pin);
    gpio_set_dir(btn_pin, GPIO_IN);
    gpio_pull_up(btn_pin);
  
    printf("Iniciou o programa.\n");
    printf("Testando o Led (este deve piscadar uma vez).\n");
    liga_led(true);
    sleep_ms(500);
    liga_led(false);

    add_repeating_timer_ms(50, repeating_callback, NULL, &timer_btn); // Aqui define o Timer de 50ms em repetição
                                                                  // O CallBack que verifica e faz a contagem do botão

    while(1) {
        sleep_ms(20000);
        printf("Deu uma volta no loop. O repeating_timer está rolando solto sem interferir no loop.\n");
    }

    return 0;
}

/**
 * @brief Liga/Desliga o Pisca-Pisca 
 * Esta função faz piscar o led na cor branca e registra se 
 * o pisca pisca está ligado ou desligado.
 * @param[in] Boolean definindo se é para ligar ou desligar o pisca.
 * @param[out] void
 */
void ligaPisca(bool liga) {
    pisca_ligado = liga;  
    if (liga == true) {
        add_repeating_timer_ms(1000/freq_piscagem, repeating_callback_pisca_led, NULL, &timer_blink);
    } else {
        cancel_repeating_timer(&timer_blink);
    }
    liga_led(liga);
    printf("Pisca-Pisca %s.\n", (liga ? "ligado" : "desligado"));
}

/**
 * @brief Inverte o estado do led
 */
void led_inverter() {
    liga_led(!led_ligado);
}

/**
 * @brief Liga/Desliga o led 
 * Esta função faz piscar o led na cor branca e registra se 
 * o pisca pisca está ligado ou desligado.
 * @param[in] Boolean definindo se é para ligar ou desligar o pisca.
 * @param[out] void
 */
void liga_led(bool liga) {
    led_ligado = liga;
    gpio_put(red, liga);
    gpio_put(blue, liga);
    gpio_put(green, liga);
}