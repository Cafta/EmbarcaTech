/*
    EMBARCATECH Program 2025
    Phase 2
    Challenge 1

    Make a Virtual Galton Board with BitDogLab 
*/

#include "Desafio_01_Galton_Board.h"

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
uint8_t balls_on_column[7] = {0, 0, 0, 0, 0, 0, 0}; // Number of balls on each column
volatile bool game_over_flag = false; // Game over flag
volatile bool game_pause_flag = false; // Game pause flag
uint16_t total_balls = 0; // Total number of balls released

int main() {
    stdio_init_all();
    button_init(); // Initialize buttons A and B

    // Iniciando o Display:
    i2c_init(i2c1, ssd1306_i2c_clock * 1000); // Initialise I2C on 400MHz (defined in ssd1306_ic2.h)
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // SDA
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // SCL
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();  // OLED initialization commands defined in ssd1306_i2c.c
    calculate_render_area_buffer_length(&frame_area); // function in ssd1306_i2c.c
    clear_ssd1306_i2c();   // clear OLED display

    // Habilita IRQ para bordas de descida (button press) e subida (release)
    gpio_set_irq_enabled_with_callback(BTN_A,
        GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
        true,
        &gpio_callback);
    gpio_set_irq_enabled_with_callback(BTN_B,
        GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
        true,
        &gpio_callback);

    multicore_launch_core1(core1_entry);

    while (true)
    {
        if (multicore_fifo_rvalid()) {
            uint32_t signal = multicore_fifo_pop_blocking();
            if (signal == PAUSE_TOGGLED) {
                // It is for synchronizing the flags with core 1
            }
        }

        if (!game_over_flag && !game_pause_flag) {
            // triggers the core 1 to release the ball
            multicore_fifo_push_blocking(random_direction()); // Send a message to core 1 to release a ball
            sleep_ms(60000/bpm); // Wait for the next ball release (in milliseconds)
        }
    }
    return 0;
}

// CORE 1 will handle the balls as they fall through the Galton Board
void core1_entry() {
    print_background();
    sleep_ms(500); // Wait for the display to be ready 
    // {33, 14}, {33, 16} Initials positions of the balls
    uint8_t wait_for_frame_ms = 1000/fps; // Wait time for each frame in milliseconds
    struct ball b[MAX_BALLS]; // Array of balls
    for (int i = 0; i < MAX_BALLS; i++) {
        b[i].active = false; // Initialize all balls as inactive
    }

    while (true) {
        // Check if there is a boll requesting to be released
        if (multicore_fifo_rvalid()) {
            // Read the direction from the FIFO queue
            uint32_t x = multicore_fifo_pop_blocking();
            if (x == A_PRESSED) { // If the value is 2, it means button A was pressed
                print_all_the_balls(ssd, b, false); // Erase all the balls from the screen
                reset_balls(b); // Reset all balls
                game_restart(); // Restart the game 
            } else if (x == B_PRESSED) { // If the value is 3, it means the Button B was pressed
                game_pause(); // Pause the game
            } else if (x == 0 || x == 1) { // If the value is 0 or 1, it means a ball is being released
                // find the first inactive ball
                for (int i = 0; i < MAX_BALLS; i++) {
                    if (!b[i].active) {
                        b[i].active = true; // Mark the ball as active
                        b[i].direction = x; // Set the direction of the ball
                        b[i].pos[0] = 39; // Initial X position
                        b[i].pos[1] = 12; // Initial Y position
                        b[i].adjust_position = 0; // Set the adjustment stage to 0 
                        total_balls++; // Increment the total number of balls released
                        print_background_top(); // Refresh the top part of the display1 
                        break;
                    }
                }
            }
        }

        // Check if the game is over
        if (game_over_flag) {
            print_all_the_balls(ssd, b, false); // Erase all the balls from the screen
            reset_balls(b); // Reset all balls
            continue;; // skip the rest of the loop if the game is over
        }
        if (game_pause_flag) {
            sleep_ms(wait_for_frame_ms); // Wait for the next frame if the game is paused
            continue; // Skip the rest of the loop if the game is paused
        }

        // Update the position of all active balls
        update_all_the_balls(ssd, b);
        sleep_ms(wait_for_frame_ms); // Wait for the next frame
    }
}

void gpio_callback(uint gpio, uint32_t events) {
    static uint32_t last_click_A = 0;
    static uint32_t last_click_B = 0;

    uint32_t current_time = to_ms_since_boot(get_absolute_time()); // Get the current time in milliseconds
    uint32_t *last_time = (gpio == BTN_A) ? &last_click_A
                         : (gpio == BTN_B) ? &last_click_B
                         : NULL;
    if (!last_time) return;  // ignore other GPIOs
    if (now() - *last_time < DEBOUNCE_MS) return; // Ignore if within debounce time
    *last_time = now();  // Store current time in last_click_A or last_click_B (selected earlier)

    if ((events & GPIO_IRQ_EDGE_FALL) && gpio == BTN_A) {
         // Button A pressed
        if (game_over_flag || game_pause_flag) {
            multicore_fifo_push_blocking(A_PRESSED); // Send a message to core 1 handle it
        }
    }

    if ((events & GPIO_IRQ_EDGE_FALL) && gpio == BTN_B) {
        // Button B pressed
        multicore_fifo_push_blocking(B_PRESSED); // Send a message to core 1 handle it
    }
}

// ################## START OF THE ACCESSORY FUNCTIONS ##################


void print_ball(uint8_t *ssd, struct ball *b, bool set) {
    // check if the ball is within the screen bounds
    if (b->pos[0] < 1 || b->pos[0] >= ssd1306_width - 1 || b->pos[1] < 1 || b->pos[1] >= ssd1306_height - 1) {
        return; // If the ball is out of bounds, do not draw it
    }
    // Draw the ball on the screen
    ssd1306_set_pixel(ssd, b->pos[0],     b->pos[1],     set);
    ssd1306_set_pixel(ssd, b->pos[0] + 1, b->pos[1],     set);
    ssd1306_set_pixel(ssd, b->pos[0] - 1, b->pos[1],     set);
    ssd1306_set_pixel(ssd, b->pos[0],     b->pos[1] + 1, set);
    ssd1306_set_pixel(ssd, b->pos[0],     b->pos[1] - 1, set);
}

void update_all_the_balls(uint8_t *ssd, struct ball *b) {
    // print_all_the_balls(ssd, b, false); // Erase all the balls from the screen
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
    // print_all_the_balls(ssd, b, true); // Draw all the balls on the screen
    render_on_display(ssd, &frame_area);
}

// clears the display completely
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
        "  GALTON BOARD - XXX ",
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

    render_on_display(ssd, &frame_area); // Atualiza o display com o conteúdo do framebuffer
    print_background_top(); // Draw the top part of the background with the bpm and fps values
}

// DRAW A VERTICAL BAR
void ssd1306_draw_vertical_bar(uint8_t *ssd, int Xi, int Xf, int Yi, int Yf, bool set) {
    if (Xi < 0 || Xf >= ssd1306_width || Yi < 0 || Yf < 0 || Yi >= ssd1306_height || Yf >= ssd1306_height) {
        return; // Check if the coordinates are within the screen bounds
    }
    for (int x = Xi; x <= Xf; x++) {
        for (int y = Yi; y <= Yf; y++) {
            ssd1306_set_pixel(ssd, x, y, set); // turn on/off the pixels
        }
    }
}

// void print_ball(uint8_t *ssd, struct ball *b, bool set) {
//     // check if the ball is within the screen bounds
//     if (b->pos[0] < 1 || b->pos[0] >= ssd1306_width - 1 || b->pos[1] < 1 || b->pos[1] >= ssd1306_height - 1) {
//         return; // If the ball is out of bounds, do not draw it
//     }
//     // Draw the ball on the screen
//     ssd1306_set_pixel(ssd, b->pos[0], b->pos[1], set);
//     ssd1306_set_pixel(ssd, b->pos[0]+1, b->pos[1], set);
//     ssd1306_set_pixel(ssd, b->pos[0]-1, b->pos[1], set);
//     ssd1306_set_pixel(ssd, b->pos[0], b->pos[1]+1, set);
//     ssd1306_set_pixel(ssd, b->pos[0], b->pos[1]-1, set);

//     // render_on_display(ssd, &frame_area); // Refreshes the display using the current framebuffer contents
// }

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
            if (balls_on_column[i] >= 50) {
                game_over();
            }
            print_bar(i); // Update the bar for the column
            break;
        }
    }
}

bool random_direction() {
    return rand() % 2; // Randomly set the direction of the ball (0 or 1)
}


// Pause function
void game_pause() {
    game_pause_flag = !game_pause_flag; // Set the game over flag to false
    if (game_pause_flag) {
        // Pause the game
        oled_draw_string(ssd, 0, 0, " A-restart | B-resume", false); // Display "PAUSE" message
        render_on_display(ssd, &frame_area); // Refreshes the display using the current framebuffer contents
    } else {
        // Resume the game
        print_background_top(); // Refresh the top part of the display
    }
}

// Game over function
void game_over() {
    game_over_flag = true; // Set the game over flag to true
    oled_draw_string(ssd, 0, 0, "   (A) to restart    ", false); // Display "GAME OVER" message
    render_on_display(ssd, &frame_area); // Refreshes the display using the current framebuffer contents
}

void print_background_top() {
    char str_total_balls[22];
    snprintf(str_total_balls, sizeof(str_total_balls), "  GALTON BOARD - %03d ", total_balls); // Format the total balls string
    oled_draw_string(ssd, 0, 0, str_total_balls, false);
    render_on_display(ssd, &frame_area); // Atualiza o display com o conteúdo do framebuffer
}

void button_init() {
    gpio_init(BTN_A); // Initialize button A
    gpio_set_dir(BTN_A, GPIO_IN); // Set button A as input
    gpio_pull_up(BTN_A); // Enable pull-up resistor for button A

    gpio_init(BTN_B); // Initialize button B
    gpio_set_dir(BTN_B, GPIO_IN); // Set button B as input
    gpio_pull_up(BTN_B); // Enable pull-up resistor for button B
}

void game_restart() {
    game_over_flag = false; // Reset the game over flag
    game_pause_flag = false; // Reset the game pause flag
    total_balls = 0; // Reset the total number of balls released
    for (int i = 0; i < 7; i++) balls_on_column[i] = 0; // Reset the number of balls in each column
    print_background(); // clear the background
}

uint32_t now() {
    // Get the current time in milliseconds
    return to_ms_since_boot(get_absolute_time());
}

/**
 * Make oll active balls inactive
 * @param b Array of balls
 * @return void 
 */
void reset_balls(struct ball *b) {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (b[i].active) {
            b[i].active = false;
        }
    }
}

/**
 * Draw all the balls on the screen
 * @param ssd Pointer to the framebuffer
 * @param b Array of balls
 * @param set Set or clear the pixels
 * @return void
 */
void print_all_the_balls(uint8_t *ssd, struct ball *b, bool set) {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (b[i].active) {
            // só desenha no framebuffer
            ssd1306_set_pixel(ssd, b[i].pos[0],     b[i].pos[1],     set);
            ssd1306_set_pixel(ssd, b[i].pos[0] + 1, b[i].pos[1],     set);
            ssd1306_set_pixel(ssd, b[i].pos[0] - 1, b[i].pos[1],     set);
            ssd1306_set_pixel(ssd, b[i].pos[0],     b[i].pos[1] + 1, set);
            ssd1306_set_pixel(ssd, b[i].pos[0],     b[i].pos[1] - 1, set);
        }
    }
    render_on_display(ssd, &frame_area); // só atualiza o display 1 vez
}