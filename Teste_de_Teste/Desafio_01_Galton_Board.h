// INCLUDES
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "inc/font6x8_com_pos_espaco.h"  // The oled_draw... functions rely on the size of these fonts
#include <string.h>  // required for memset()

// MAIN FUNCTIONS
void core1_entry(); // Function to be executed by core 1
void gpio_callback(uint gpio, uint32_t events); // GPIO callback function for button press and release

// Accessories Functions
void clear_ssd1306_i2c();
void oled_draw_char(uint8_t *ssd, int16_t x, int16_t y, uint8_t character, bool invert) ;
void oled_draw_string(uint8_t *ssd, int16_t x, int16_t y, const char *string, bool invert);
void print_background();
void print_background_top(); // Draws the top part of the background with the bpm and fps values
void ssd1306_draw_vertical_bar(uint8_t *ssd, int Xi, int Xf, int Yi, int Yf, bool set); 
void print_ball(uint8_t *ssd, struct ball *b, bool set); // Draws a ball on the screen
bool random_direction(); // Randomly sets the direction of the ball
void columns_update(struct ball b); // Updates the number of balls in the column
void game_over(); // Game over function
void game_restart(); // Restarts the game
void game_pause(); // Pauses the game
void button_init(); // Initializes the buttons
uint32_t now(); // Gets the current time in milliseconds
void reset_balls(struct ball *b); // Resets the balls on the screen
void print_all_the_balls(uint8_t *ssd, struct ball *b, bool set); // Draws all the balls on the screen
void update_all_the_balls(uint8_t *ssd, struct ball *b);

// HARDWARE DEFINITIONS
#define SSD1306_I2C_CLOCK 400 // I2C clock speed in kHz
#define SSD1306_I2C_ADDR 0x3C
#define I2C_SDA 14
#define I2C_SCL 15
#define BTN_A 5 // Button A GPIO pin
#define BTN_B 6 // Button B GPIO pin

// SOFTWARE DEFINITIONS
#define MAX_BALLS 32 // Maximum number of balls
int8_t adjust_positions[2][3] = { // Downward adjustments for the balls
    {3, 2, 1},  // X
    {2, 2, 4}   // Y
};
#define A_PRESSED 2 // Button A pressed
#define B_PRESSED 3 // Button B pressed 
#define PAUSE_TOGGLED 4 // Pause toggled
#define DEBOUNCE_MS 80 // Debounce time in milliseconds
#define bpm 70 // Number of balls per minute
#define fps 15 // Frames per second