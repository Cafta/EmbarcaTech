#include "led_embutido.h"
#include <stdbool.h>

int led_embutido_init() {
    return init_led_embutido();
}

void led_embutido_toggle() {
    static bool led_state = false; // Variável estática para armazenar o estado do LED
    led_state = !led_state; // Inverte o estado do LED
    set_led_embutido(led_state); // Atualiza o LED com o novo estado
}