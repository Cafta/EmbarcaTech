#include "pico/stdlib.h"
#include <stdbool.h>
#include <stdio.h>

#define red 13
#define blue 12
#define green 11
#define btn_pin 6

void ligaLed(bool liga);
volatile bool led_ligado[] = {false, true}; // Maquina de estados para evitar comando repetido
    // Toda variável alterada fora do processamento principal (tipo em uma instrução de interrupção) deve
    // receber o qualificador 'volatile' para que o processador não utilize o valor do cach da variável
absolute_time_t turn_off_time; 

int64_t callback() {
    led_ligado[0] = false; // Optei por só trocar uma variável a desligar direto as saídas para não demorar na função
    return 0; 
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

    bool botao_apertado[] = {1, 0}; // Maq de estados para evitar disparo do botão

  while(1) {
    botao_apertado[0] = gpio_get(btn_pin);  
    sleep_ms(20);  // evitar bouncing
    if (botao_apertado[0] != botao_apertado[1]) { // só entra se o estado atual for diferente do último estado
        botao_apertado[1] = botao_apertado[0]; // registra a alteração de estado (só entra novamente se mudar novamente o estado) 
        if (botao_apertado[0] == 0 && led_ligado[0] == false) {  // não entra se soltou o botão, só quando aperta, e só se o led estiver desligado
            add_alarm_in_ms(3000, callback, NULL, false); // (tempo em ms, nome da função, passagem de variaveis, mantém ligado);
            led_ligado[0] = true; // liga o led
        }
    } 
    printf("led_ligado(%d, %d)\n", led_ligado[0], led_ligado[1]);
    if (led_ligado[0] != led_ligado[1]) { // houve mudança de estados
        led_ligado[1] = led_ligado[0]; // registra a alteração de estado (só entra novamente se mudar novamente o estado) 
        ligaLed(led_ligado[0]); // executa a mudança;
    }
  }

  return 0;
}

void ligaLed(bool liga) {
      gpio_put(red, liga);
      gpio_put(blue, liga);
      gpio_put(green, liga);  
}