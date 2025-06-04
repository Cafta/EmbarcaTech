#include "pico/stdlib.h"
#include "ssd1306.h"
#include "hardware/i2c.h"
#include "font6x8_com_pos_espaco.h"  // The oled_draw... functions rely on the size of these fonts
#include <string.h>  // required for memset()

// Definição de funções principais
void print_page(uint8_t codigo_pg);

// Definição de funções acessórias:
void oled_init();
void ssd1306_init(); // esta está em ssd1306_i2c.c (não é de minha autoria) 
void clear_ssd1306_i2c();
void oled_draw_char(int16_t x, int16_t y, uint8_t character, bool invert) ;
void oled_draw_string(int16_t x, int16_t y, const char *string, bool invert);
void print_background();
void print_oled(int16_t x, int16_t y, const char *string, bool invert);
void render();