#include "pico/stdlib.h"
#include <stdbool.h>
#include <stdio.h>

#define red 13
#define blue 12
#define green 11
#define btn_pin 6

void ligaLed(bool liga);

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

  while(1) {
    bool btn = gpio_get(btn_pin);  
    sleep_ms(20);
    printf("botao = %d", btn);
    while(gpio_get(btn_pin) == 0){ 
      ligaLed(true);
    }
    ligaLed(false);
    sleep_ms(1000);
    ligaLed(true);
    sleep_ms(1000);
  }
  return 0;
}

void ligaLed(bool liga) {
      gpio_put(red, liga);
      gpio_put(blue, liga);
      gpio_put(green, liga);  
}