/*
    EMBARCATECH Program 2025
    Phase 2
    Challenge 1

    Make a Virtual Galton Board with BitDogLab 
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "inc/font6x8_com_pos_espaco.h"  // The oled_draw... functions rely on the size of these fonts
#include <string.h>  // required for memset()

// HARDWARE DEFINITIONS
#define SSD1306_I2C_CLOCK 400 // I2C clock speed in kHz
#define SSD1306_I2C_ADDR 0x3C
#define I2C_SDA 14
#define I2C_SCL 15

// SOFTWARE DEFINITIONS
#define MAX_BALLS 32 // Maximum number of balls
int8_t adjust_positions[2][3] = { // Downward adjustments for the balls
    {3, 2, 1},  // X
    {2, 2, 4}   // Y
};

// STRUCTS DEFINITIONS
struct render_area frame_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1
};
struct ball {
    int pos[2]; // position (x, y);
    int direction; // ball directions (1 = right, 0 = left)
    int adjust_position; // stage of adjustment
    bool active; // ball active status (true = active, false = inactive)
};
  
// Global framebuffer for the SSD1306 OLED display
uint8_t ssd[ssd1306_buffer_length];

// GLOBAL VARIABLES
uint8_t bpm = 60; // Number of balls per minute
uint8_t fps = 25; // Frames per second
uint8_t balls_on_column[] = {0, 0, 0, 0, 0, 0, 0}; // Number of balls on each column
bool game_over_flag = false; // Game over flag

// Accessories Functions
void clear_ssd1306_i2c();
void oled_draw_char(uint8_t *ssd, int16_t x, int16_t y, uint8_t character, bool invert) ;
void oled_draw_string(uint8_t *ssd, int16_t x, int16_t y, const char *string, bool invert);
void print_background();
void ssd1306_draw_vertical_bar(uint8_t *ssd, int Xi, int Xf, int Yi, int Yf, bool set); 
void print_ball(uint8_t *ssd, struct ball *b, bool set); // Draws a ball on the screen
void update_ball(struct ball *b); // Updates the ball position on the screen
bool random_direction(); // Randomly sets the direction of the ball
void columns_update(struct ball b); // Updates the number of balls in the column
void game_over(); // Game over function (not implemented in this code)

// CORE 1 will handle the balls as they fall through the Galton Board
void core1_entry() {
    // {33, 14}, {33, 16} Initials positions of the balls
    uint8_t wait_for_frame_ms = 1000/fps; // Wait time for each frame in milliseconds
    struct ball b[MAX_BALLS]; // Array of balls
    for (int i = 0; i < MAX_BALLS; i++) {
        b[i].active = false; // Initialize all balls as inactive
    }

    while (true) {
        // Check if the game is over
        // if (game_over_flag) {
        //     for (int i = 0; i < MAX_BALLS; i++) {
        //         if (b[i].active) {
        //             print_ball(ssd, &b[i], false); // Erase the ball from the old position
        //             b[i].active = false; // Mark the ball as inactive
        //         }
        //     }
        //     break; // Exit the loop if the game is over
        // }

        // Check if there is a boll requesting to be released
        if (multicore_fifo_rvalid()) {
            // Read the direction from the FIFO queue
            uint32_t x = multicore_fifo_pop_blocking();
            // find the first inactive ball
            for (int i = 0; i < MAX_BALLS; i++) {
                if (!b[i].active) {
                    b[i].active = true; // Mark the ball as active
                    b[i].direction = x; // Set the direction of the ball
                    b[i].pos[0] = 39; // Initial X position
                    b[i].pos[1] = 12; // Initial Y position
                    b[i].adjust_position = 0; // Set the adjustment stage to 0  
                    break;
                }
            }
        }

        // Update the position of all active balls
        for (int i = 0; i < MAX_BALLS; i++) {
            if (b[i].active) {
                print_ball(ssd, &b[i], false); // Erase the ball from the old position
                
                // There is 4 parts of this section:
                // 1. When the ball is starting to fall (make a straight line until 17)
                // 2. When the ball is falling down the Galton Board
                // 3. When the ball is at the gathering end of the Galton Board (make a straight line after 48)
                // 4. When the ball get out of the screen (mark it as inactive)
                
                if (b[i].pos[1] < 16) { // starting to fall
                    b[i].pos[1] += 2; // Move the ball down by 2 pixels
                } else if (b[i].pos[1] < 60) { // falling down the Galton Board
                    int ap = b[i].adjust_position++; // Get the current adjustment position and increment it
                    if (ap > 2) {
                        ap = 0; // reset the adjustment position if it exceeds 2
                        b[i].adjust_position = 1; // reset the adjustment position
                        b[i].direction = random_direction(); // Randomly redirect the ball
                    }
                    b[i].pos[1] += adjust_positions[1][ap]; // Move the ball down by the Y adjustment value
                    if (b[i].direction == 0) { 
                        b[i].pos[0] -= adjust_positions[0][ap]; // Move the ball left by the X adjustment value
                    } else {
                        b[i].pos[0] += adjust_positions[0][ap]; // Move the ball right by the X adjustment value
                    }
                } else if (b[i].pos[1] < 63) { // in the gathering end of the Galton Board
                    b[i].pos[1] += 2; // Move the ball down by 2 pixels
                } else { // reached the end of the screen
                    b[i].active = false; // Mark the ball as inactive
                    columns_update(b[i]); // Update the number of balls in the column
                }
                if (b[i].active == true) print_ball(ssd, &b[i], true); // Draw the ball at the new position
            }
        }
        
        sleep_ms(wait_for_frame_ms); // Wait for the next frame
    }
}

int main() {
    stdio_init_all();

    // Iniciando o Display:
    i2c_init(i2c1, ssd1306_i2c_clock * 1000); // Initialise I2C on 400MHz (defined in ssd1306_ic2.h)
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // SDA
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // SCL
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();  // OLED initialization commands defined in ssd1306_i2c.c
    calculate_render_area_buffer_length(&frame_area); // function in ssd1306_i2c.c
    clear_ssd1306_i2c();   // clear OLED display

    multicore_launch_core1(core1_entry);

    print_background(); 

    // Creating a example of a ball
    // struct ball b1 = { {33, 14}, 0 }; // Ball position (x, y) and direction (0 = left, 1 = right)
    // update_ball(&b1); // Draw the ball on the screen

    // sleep_ms(5000); // para dar tempo para ligar o Serial-Monitor

    while (true)
    {
        // triggers the core 1 to release the ball
        multicore_fifo_push_blocking(random_direction()); // Send a message to core 1 to release a ball
        sleep_ms(60000/bpm); // Wait for the next ball release (in milliseconds)
    }
    return 0;
}

// Apaga o display totalmente
void clear_ssd1306_i2c() {
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}
  
// Desenha um único caractere no display
void oled_draw_char(uint8_t *ssd, int16_t x, int16_t y, uint8_t character, bool invert) {
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
        oled_draw_char(ssd, x, y, *string++, invert);
        x += 6;
    }
}

void print_background() {
    uint8_t coordenada_Y = 0;
    clear_ssd1306_i2c();

    const char PG_INITIAL[8][22] = {  // 8 linhas com 21 caracteres + 1 para `\0`
        "  00 bpm  |  00 fps  ",
        "---------------------",
        "      o              ",
        "     o o             ",
        "    o o o            ",
        "   o o o o           ",
        "  o o o o o          ",
        " o o o o o o         "
    }; 
    
    for (int linha = 0; linha < 8; linha++) {
        coordenada_Y = linha * 8;
        oled_draw_string(ssd, 0, coordenada_Y, PG_INITIAL[linha], false);
    }

    char strfps[3];
    char strbpm[3];
    snprintf(strbpm, sizeof(strbpm), "%02d", bpm); // Convert bpm to string
    snprintf(strfps, sizeof(strfps), "%02d", fps); // Convert fps to string
    oled_draw_string(ssd, 2*6, 0, strbpm, false);
    oled_draw_string(ssd, 13*6, 0, strfps, false);

    render_on_display(ssd, &frame_area); // Atualiza o display com o conteúdo do framebuffer
}

// DESENHA UMA BARRA VERTICAL
void ssd1306_draw_vertical_bar(uint8_t *ssd, int Xi, int Xf, int Yi, int Yf, bool set) {
    if (Xi < 0 || Xf >= ssd1306_width || Yi < 0 || Yf < 0 || Yi >= ssd1306_height || Yf >= ssd1306_height) {
        return; // Verifica se os parâmetros estão dentro dos limites
    }
    for (int x = Xi; x <= Xf; x++) {
        for (int y = Yi; y <= Yf; y++) {
            ssd1306_set_pixel(ssd, x, y, set); // Acende pixel na posição (x, y)
        }
    }
}

void print_ball(uint8_t *ssd, struct ball *b, bool set) {
    // Draw the ball on the screen
    ssd1306_set_pixel(ssd, b->pos[0], b->pos[1], set);
    ssd1306_set_pixel(ssd, b->pos[0]+1, b->pos[1], set);
    ssd1306_set_pixel(ssd, b->pos[0]-1, b->pos[1], set);
    ssd1306_set_pixel(ssd, b->pos[0], b->pos[1]+1, set);
    ssd1306_set_pixel(ssd, b->pos[0], b->pos[1]-1, set);

    render_on_display(ssd, &frame_area); // Refreshes the display using the current framebuffer contents
}

void print_bar(int column) {
    // Draw the vertical bar for the column
    int offset = 13*6+3;
    int Xi = offset + column * 6; // Calculate the X position of the bar
    int Xf = offset + column * 6 + 3; // Calculate the X position of the bar
    int Yi = 62 - balls_on_column[column]; // Calculate top position of the bar
    int Yf = 62; // calculate bottom position of the bar

    ssd1306_draw_vertical_bar(ssd, Xi, Xf, Yi, Yf, true); // Draw the bar on the screen
    render_on_display(ssd, &frame_area); // Refreshes the display using the current framebuffer contents
}

void columns_update(struct ball b) {
    for (int i = 0; i < 7; i++) {
        if (b.pos[0] > i*12 && b.pos[0] < i*12+6) { // Check if the ball is in the column range
            balls_on_column[i]++;
            if (balls_on_column[i] > 52) {
                game_over();
            }
            print_bar(i); // Update the bar for the column
        }
    }
}

bool random_direction() {
    return rand() % 2; // Randomly set the direction of the ball (0 or 1)
}

void game_over() {
    // Game over function (not implemented in this code)
    game_over_flag = true; // Set the game over flag to true
    // clear_ssd1306_i2c(); // Clear the display
    oled_draw_string(ssd, 0, 0, "      GAME OVER      ", false); // Display "GAME OVER" message
    render_on_display(ssd, &frame_area); // Refreshes the display using the current framebuffer contents
    sleep_ms(2000); // Wait for 2 seconds before exiting
}