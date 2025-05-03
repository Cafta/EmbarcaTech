#include "pico/stdlib.h"  
#include "hal_led.h" 

int main(){
    stdio_init_all(); // Inicializa a comunicação serial
    if (led_embutido_init()) return -1; // Inicializa o LED embutido

    while (1) {
        led_embutido_toggle(); // Alterna o estado do LED embutido
        sleep_ms(500); // Aguarda 500 ms
    }

    return 0; // Retorna 0 ao sistema operacional (embora não seja alcançado neste caso)
}