#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdbool.h>
#include <stdio.h>

#define red 13
#define blue 12
#define green 11
#define btn_pin 6

int turn_off_time_ms = 2000;
void ligaLed(bool liga);
bool led_ligado = false; // Maquina de estados para evitar comando repetido
bool botao_apertado = 1;
int btn_count = 0;

int64_t turn_off_callback(alarm_id_t id, void *user_data) {
   ligaLed(false);
   led_ligado = false;
   return 0;
}

bool repeating_callback(struct repeating_timer *t) {
    // A cada período de repetição (100ms) ele repete esta instrução, deixando livre o processador nos restante
    // do período para realizar a tarefa do while() lá no main();
    // isso pode funcionar como uma "atividade paralela" ao programa principal. Mas ele também consome processamento,
    // o que indica que pode deixar o programa mais lento se for uma função muito pesada.

    botao_apertado = gpio_get(btn_pin);
    // aqui eu não preciso de uma estratégia de debouncing, porque a função só roda a cada 100ms. Então analisa
    // a situação do botão precisamente neste momento, sem risco de registrar vários cliques seguidos.

    if (botao_apertado == 0 && led_ligado == false) { // só entra se o botão está apertado E o led está desligado
        ligaLed(true);    // liga o led, ele não entra mais nesse if
        add_alarm_in_ms(turn_off_time_ms, turn_off_callback, NULL, false); // Cria o alarme para desligar o led
    } 

    return true;
}

int main() {
    stdio_init_all();
    gpio_init(red);
    gpio_set_dir(red, GPIO_OUT);
    gpio_init(blue);
    gpio_set_dir(blue, GPIO_OUT);
    gpio_init(green);
    gpio_set_dir(green, GPIO_OUT);
    gpio_init(btn_pin);
    gpio_set_dir(btn_pin, GPIO_IN);
    gpio_pull_up(btn_pin);
  
    printf("Iniciou o programa\n");
    ligaLed(true);
    sleep_ms(500);
    ligaLed(false);

    struct repeating_timer timer;
    add_repeating_timer_ms(20, repeating_callback, NULL, &timer); // Aqui define o Timer de 20ms
                                                                    // Ele fica repetindo, liberando o while

    while(1) {
        sleep_ms(10000);
        printf("Deu uma volta no loop. O Timer está rolando solto sem interferir no loop.\n");
    }

    return 0;
}

void ligaLed(bool liga) {
    led_ligado = liga;
    gpio_put(red, liga);
    gpio_put(blue, liga);
    gpio_put(green, liga);  
}