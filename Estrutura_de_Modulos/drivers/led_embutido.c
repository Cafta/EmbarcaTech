/*
    Driver para controlar um LED embutido na placa específica do RaspBerryPi Pico.
    É considerado um driver porque só funciona com o hardware do controlador do RaspBerryPi Pico.
    O driver é específico para o RaspBerryPi Pico, enquanto o Hal seria uma camada de abstração que poderia ser usada em diferentes microcontroladores.
*/

/*
   Driver para o LED embutido da Raspberry Pi Pico W
   (usa o LED controlado pelo chip CYW43).
*/
#include <stdbool.h>
#include "pico/stdlib.h"          // traz as definições de board
#include "pico/cyw43_arch.h"      // API do CYW43

/* Se o LED do CYW43 não existir, abortamos a compilação
– isso cobre qualquer placa que não seja Pico W ou derivada. */
#ifndef CYW43_WL_GPIO_LED_PIN
#  error "CYW43_WL_GPIO_LED_PIN não definido: este driver requer uma Raspberry Pi Pico W (ou placa compatível com CYW43)."
#endif

int init_led_embutido() {
    return cyw43_arch_init(); // Inicializa a arquitetura CYW43
}

void set_led_embutido(bool led_state) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state); // Atualiza o LED com o novo estado
}
