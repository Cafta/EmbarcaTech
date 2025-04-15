/*
    EMBARCATECH Program 2025
    Phase 2
    Exercise 1

    I started putting everything in English in the middle of this exercise, so you will
    find some expressions in Portuguese in this one. 
*/
#include <stdio.h>
#include <stdint.h>  // Required for sprintf()
#include "pico/stdlib.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "ws2812.pio.h"
#include <string.h>  // Required for memset()

// Accessory libraries
#include "inc/led_matrix_numbers.h"  // Number patterns for LED matrix
#include "inc/font6x8_com_pos_espaco.h"  // Characters patterns (8x6) for OLED 

// HARDWARE definitions
#define BTN_A 5  // A Button
#define BTN_B 6  // B Button
#define MATRIX_PIN 7  // Matrix 5x5 LED
#define SSD1306_I2C_ADDR 0x3C  // Display OLED
#define I2C_SDA 14   // Display OLED
#define I2C_SCL 15   // Display OLED

// SISTEM definitions
#define BOUNCING_BLOCK 10  // time (in ms) to avoid bouncing
#define TIME_SITE 0
#define CLICK_SITE 1
#define MAX_SCORE_SITE 2

// STRUCTS Definitions
struct render_area frame_area = {  // for Display OLED
  .start_column = 0,
  .end_column = ssd1306_width - 1,
  .start_page = 0,
  .end_page = ssd1306_n_pages - 1
};

// Global display Buffer for SSD1306 OLED 
uint8_t ssd[ssd1306_buffer_length];

// System variable
PIO pio = pio0;
uint sm;
bool cronometro = false;
bool btn_b_ativo = false; // If Button B is currently active
uint8_t btn_b_pressed = 0;  // Number of times button B has been pressed.
struct repeating_timer general_timer;
uint8_t record = 0;

// Main functions
void button_callback(uint gpio, uint32_t events);
bool repeating_callback(struct repeating_timer *t);
int64_t clear_callback();
void contagem_regressiva();

// Accessory functions
void ssd1306_init(); // It's in ssd1306_i2c.c (not mine) 
void clear_ssd1306_i2c();
void OLED_draw_char(uint8_t *ssd, int16_t x, int16_t y, uint8_t character, bool invert) ;
void oled_draw_string(uint8_t *ssd, int16_t x, int16_t y, const char *string, bool invert);
void print_menu();
void display_init();
void btn_init();
void matrix_init();
void ws2812_send(uint32_t color);
void set_ws2812(const uint32_t colors[numero_de_leds]);
void print_OLED(char n[4], uint8_t x, uint8_t y);
void print_number(uint8_t number, uint8_t SITE);

// ************ START HERE ***************
int main() {
    stdio_init_all();
    btn_init();
    matrix_init();
    display_init();

    print_menu();

  while(true) {
    sleep_ms(100000);  // Este aqui "hiberna" o processador, só "desperta" se ocorrer uma interrupção. Economiza mais energia
  }

  return 0;
}


// *****************  MAIN FUNCTIONS START FROM HERE  ********************

void button_callback(uint gpio, uint32_t events) {
    static absolute_time_t last_click = 0;
    if (events == GPIO_IRQ_EDGE_FALL) {
        absolute_time_t now = get_absolute_time();
        int16_t spent_time = absolute_time_diff_us(last_click, now) / 1000;
        if (spent_time > BOUNCING_BLOCK) {
            if (gpio == BTN_A) {
                // printf("Button A pressed\n");
                if (!cronometro) {
                    cronometro = true;
                    btn_b_pressed = 0;
                    print_number(btn_b_pressed, CLICK_SITE);
                    contagem_regressiva();
                    add_repeating_timer_ms(1000, repeating_callback, NULL, &general_timer);
                }
            } else if (gpio == BTN_B) {
                // printf("Button B pressed\n");
                if (btn_b_ativo) {
                    btn_b_pressed++;
                    print_number(btn_b_pressed, CLICK_SITE);
                }
            }
        }
    }
    last_click = get_absolute_time();
}

void contagem_regressiva() {
    static int n = 10;
    btn_b_ativo = true;
    set_ws2812(numero[--n]);
    print_number(n, TIME_SITE);
    if (n == 0) {
        // Acabou a contagem. Desliga tudo até click no btn A
        n = 10;
        if (btn_b_pressed > record) {
            record = btn_b_pressed;
            print_number(record, MAX_SCORE_SITE);
        }
        btn_b_ativo = false;
        cronometro = false;
        cancel_repeating_timer(&general_timer);
        add_alarm_in_ms(1000, clear_callback, NULL, false); // (tempo em ms, nome da função, passagem de variaveis, mantém ligado);
    } 
    // printf("btn B pressed: %d\n", btn_b_pressed);
}

bool repeating_callback(struct repeating_timer *t) {   
    contagem_regressiva();
    return true;
}

int64_t clear_callback() { 
    uint32_t black = 0x000000; // Apaga todos os LEDs
    for (int i = 0; i < numero_de_leds; i++) {
        ws2812_send(black);
    }
    return 0;
}


// *****************  ACCESSORY FUNCTIONS START FROM HERE  ********************

// Apaga o display totalmente
void clear_ssd1306_i2c() {
  memset(ssd, 0, ssd1306_buffer_length);
  render_on_display(ssd, &frame_area);
}

// Desenha um único caractere no display
void OLED_draw_char(uint8_t *ssd, int16_t x, int16_t y, uint8_t character, bool invert) {
  int fb_idx = (y / 8) * 128 + x;
  
  for (int i = 0; i < 6; i++) {
    ssd[fb_idx++] = invert? ~FONT6x8[character - 0x20][i] : FONT6x8[character - 0x20][i];
  }
}

// Desenha uma string, chamando a função de desenhar caractere várias vezes
void oled_draw_string(uint8_t *ssd, int16_t x, int16_t y, const char *string, bool invert) {
  if (x > ssd1306_width - 6 || y > ssd1306_height - 8) {
      return;
  }

  x = (x == 0) ? 1: x;

  while (*string) {
    OLED_draw_char(ssd, x, y, *string++, invert);
    x += 6;
  }
}

void print_menu() {
    // Background
    clear_ssd1306_i2c(); 

    const char menu[8][22] = {
        "    MAX SCORE: 000    ",
        "                      ",
        "  time:      clicks:  ",
        "                      ",
        "--------------------- ",
        "     A -> Start       ",
        "     B -> Points      "
    };

    uint8_t coordenada_Y = 0;
    for (int line = 0; line < 8; line++) {
        coordenada_Y = line * 8;
        oled_draw_string(ssd, 0, coordenada_Y, menu[line], false);
    }
    render_on_display(ssd, &frame_area);
}

void print_number(uint8_t number, uint8_t SITE) {
    uint8_t x, y;
    char n[4];
    sprintf(n, "%03u", number);
    if (SITE == TIME_SITE) {
        x = 18;
        y = 24;
    } else if (SITE == CLICK_SITE) {
        x = 90;
        y = 24;
    } else if (SITE == MAX_SCORE_SITE) {
        x = 90;
        y = 0;
    }
    print_OLED(n, x, y);
}

void print_OLED(char n[4], uint8_t x, uint8_t y) {
    oled_draw_string(ssd, x, y, n, false);
    render_on_display(ssd, &frame_area);
}

void display_init() {
    i2c_init(i2c1, ssd1306_i2c_clock * 1000); // Inicializa I2C a 400MHz (definido em ssd1306_ic2.h)
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // SDA
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // SCL
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();  // Comandos de Inicialização do ssd1306 definido em ssd1306_i2c.c
    calculate_render_area_buffer_length(&frame_area); // função em ssd1306_i2c.c
    clear_ssd1306_i2c();   // zera o display inteiro
}

void btn_init() {
    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);
    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);

    // Buttons IRQs
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &button_callback);
    gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &button_callback);
}

void matrix_init() {
       // Configuração do PIO para controlar matriz de LEDs
       uint offset = pio_add_program(pio, &ws2812_program);
       sm = pio_claim_unused_sm(pio, true);
       ws2812_program_init(pio, sm, offset, MATRIX_PIN, 800000, false);
}

void ws2812_send(uint32_t color) {
    pio_sm_put_blocking(pio, sm, color << 8);
}

void set_ws2812(const uint32_t colors[numero_de_leds]) {
    for (int i = 0; i < numero_de_leds; i++) {
        ws2812_send(colors[i]);
    }
}