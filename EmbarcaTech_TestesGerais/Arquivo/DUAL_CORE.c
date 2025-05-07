/*
    EMBARCATECH Program 2025
    Phase 2
    Challenge 1

    Make a Virtual Galton Board with BitDogLab 
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

void core1_entry() {
    while (true) {
        if (multicore_fifo_rvalid()) {  // verifica se tem alguma mensagem para receber
            // Espera receber valor do core 0
            uint32_t x = multicore_fifo_pop_blocking();
            printf("Core 1 recebeu: %u\n", x);
            // Envia resposta de volta (por exemplo, o dobro)
            multicore_fifo_push_blocking(x * 2);
        }
    }
}

int main() {
    stdio_init_all();

    multicore_launch_core1(core1_entry);

    sleep_ms(5000); // para dar tempo para ligar o Serial-Monitor

    for (int i = 1; i <= 10; i++) {
        printf("Core 0 enviando: %d\n", i);
        multicore_fifo_push_blocking(i);  // envia para core 1

        uint32_t resposta = multicore_fifo_pop_blocking();  // espera resposta
        printf("Core 0 recebeu: %u\n", resposta);
        sleep_ms(1000);
    }

    return 0;
}

