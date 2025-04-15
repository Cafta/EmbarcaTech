/*
    EMBARCATECH Program 2025
    Phase 2
    Exercise 2

    Map Joystick coordinates on the OLED display
*/

#include "stdio.h"
#include "pico/stdlib.h"
#include "inc/ssd1306.h"  // OLED display
#include "hardware/i2c.h"  // used in OLED
#include "inc/font6x8_com_pos_espaco.h"  // oled_draw... functions use the 6x8 pixel-sized characters
#include <string.h>  // needed for memset()
#include "hardware/adc.h"  // Required for Joystick

// Hardware definitions
#define SSD1306_I2C_ADDR 0x3C
#define I2C_PORT i2c0
#define I2C_SDA 14
#define I2C_SCL 15
#define VRY_PIN 27
#define VRX_PIN 26
#define VRY 0
#define VRX 1

// STRUCTS definitions
struct render_area frame_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1
};

// Global Buffer for the SSD1306 Display 
uint8_t ssd[ssd1306_buffer_length];

// Function declarations
void clear_ssd1306_i2c();
void OLED_draw_char(uint8_t *ssd, int16_t x, int16_t y, uint8_t character, bool invert) ;
void oled_draw_string(uint8_t *ssd, int16_t x, int16_t y, const char *string, bool invert);
void OLED_init();
void joystick_init();
int remap(int x, int in_min, int in_max, int zero, int out_min, int out_max);
void print_coordenada(); 

int main()
{
    OLED_init();
    joystick_init();

    while (true) {
        print_coordenada();
        sleep_ms(10);
    }
}

void OLED_init() {
    // Starting the Display
    i2c_init(i2c1, ssd1306_i2c_clock * 1000); // Start the I2C at 400kHz (set in ssd1306_i2c.h)
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // SDA
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // SCL
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();  // It is in ssd1306_i2c.c
    calculate_render_area_buffer_length(&frame_area); // in ssd1306_i2c.c
    clear_ssd1306_i2c();   // Resets the display
}

// Resets the display
void clear_ssd1306_i2c() {
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}
  
// Draw a single character on the display
void OLED_draw_char(uint8_t *ssd, int16_t x, int16_t y, uint8_t character, bool invert) {
    int fb_idx = (y / 8) * 128 + x;
    for (int i = 0; i < 6; i++) {
        ssd[fb_idx++] = invert? ~FONT6x8[character - 0x20][i] : FONT6x8[character - 0x20][i];
    }
}

// Draw a string by calling OLED_draw_char() for each character
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

void joystick_init() {
    // Setting the Joystick
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);
}

void print_coordenada() {
    // Reading joystick axes
    adc_select_input(VRX);
    int16_t x = adc_read();
    x = remap(x, 18, 4081, 2010, -2048, 2048);
    adc_select_input(VRY);
    int16_t y = adc_read();
    y = remap(y, 18, 4082, 1937, -2048, 2048);

    // Writing to OLED display
    char buffer[22]; 
    char cabecalho[] = "    Coordenadas:      ";
    snprintf(buffer, sizeof(buffer), "   ( %04d , %04d )    ", x, y);
    oled_draw_string(ssd, 0, 16, cabecalho, false);
    oled_draw_string(ssd, 0, 32, buffer, false);
    render_on_display(ssd, &frame_area);

}

// Apply corrections to joystick axes 
int remap(int x, int in_min, int in_max, int metade, int out_min, int out_max) {
    int ded_zone = 200;
    if (x > (metade - ded_zone) && x < (metade + ded_zone)) {
        return 0;
    } else if (x < metade) {
        return (metade - x) * out_min / (metade - in_min);
    } else if (x > metade) {
        return (x-metade) * out_max / (in_max-metade);
    }    
}