#include <stdio.h>
#include "pico/stdlib.h"
#include "ws2812.pio.h"

// Bibliotecas acessórias
#include "inc/led_matrix_numbers.h"

// DEFINIÇÕES DE HARDWARE
#define BTN_A 5  // botão A
#define BTN_B 6  // botão B
#define MATRIX_PIN 7

// DEFINIÇÕES DE SISTEMA
#define BOUNCING_BLOCK 100  // tempo em ms que bloqueia o botão para evitar bouncing

// VARIAVEIS DE SISTEMA
PIO pio = pio0;
uint sm;


// FUNÇÕES PRINCIPAIS
void button_callback(uint gpio, uint32_t events);

// FUNÇÕES ACESSÓRIAS
void set_ws2812(uint32_t colors[numero_de_leds]);
void ws2812_send(uint32_t color);

int main()
{
    // Inicia o monitor serial (computador)
    stdio_init_all();

    // Inicia os botões
    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);
    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);

    // Inicializa as Interrupções dos botões
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &button_callback);
    gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &button_callback);

    // Variáveis
    int8_t n = 9;

    // Configuração do PIO para controlar matriz de LEDs
    uint offset = pio_add_program(pio, &ws2812_program);
    sm = pio_claim_unused_sm(pio, true);
    ws2812_program_init(pio, sm, offset, MATRIX_PIN, 800000, false);

    while (true) {
        set_ws2812(numero[--n]);
        if (n == 0) n = 10;
        sleep_ms(1000);
    }
}


void button_callback(uint gpio, uint32_t events) {
    static absolute_time_t last_click = 0;
    if (events == GPIO_IRQ_EDGE_FALL) {
        absolute_time_t now = get_absolute_time();
        int16_t spent_time = absolute_time_diff_us(last_click, now) / 1000;
        if (spent_time > BOUNCING_BLOCK) {
            if (gpio == BTN_A) {
                printf("Button A pressed\n");
            } else if (gpio == BTN_B) {
                printf("Button B pressed\n");
            }
        }
    }
    last_click = get_absolute_time();
}

void set_ws2812(uint32_t colors[numero_de_leds]) {
    for (int i = 0; i < numero_de_leds; i++) {
        ws2812_send(colors[i]);
    }
}

void ws2812_send(uint32_t color) {
    pio_sm_put_blocking(pio, sm, color << 8);
}
