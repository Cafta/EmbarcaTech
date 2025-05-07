#include "pico/stdlib.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "inc/font6x8_com_pos_espaco.h"  // The oled_draw... functions rely on the size of these fonts
#include <string.h>  // required for memset()

// Definições Gerais
#define SSD1306_I2C_ADDR 0x3C
#define I2C_SDA 14
#define I2C_SCL 15
#define PAG_INICIAL_COD 1
#define PAG_PADRAO_COD 2

// STRUCTS definitions
struct render_area frame_area = {
  .start_column = 0,
  .end_column = ssd1306_width - 1,
  .start_page = 0,
  .end_page = ssd1306_n_pages - 1
};

// Global framebuffer for the SSD1306 OLED display 
uint8_t ssd[ssd1306_buffer_length];

// Definição de funções principais
void print_page(uint8_t codigo_pg);

// Definição de funções acessórias:
void ssd1306_init(); // esta está em ssd1306_i2c.c (não é de minha autoria) 
void clear_ssd1306_i2c();
void oled_draw_char(uint8_t *ssd, int16_t x, int16_t y, uint8_t character, bool invert) ;
void oled_draw_string(uint8_t *ssd, int16_t x, int16_t y, const char *string, bool invert);
void print_background();

// INÍCIO DO PROGRAMA
int main() {
  stdio_init_all();

    // Iniciando o Display:
    i2c_init(i2c1, ssd1306_i2c_clock * 1000); // Inicializa I2C a 400MHz (definido em ssd1306_ic2.h)
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // SDA
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // SCL
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();  // Comandos de Inicialização do ssd1306 definido em ssd1306_i2c.c
    calculate_render_area_buffer_length(&frame_area); // função em ssd1306_i2c.c
    clear_ssd1306_i2c();   // zera o display inteiro

    // UMA LINHA
    const char texto[] = "     Hello World      ";
    oled_draw_string(ssd, 0, 0, texto, false);
    render_on_display(ssd, &frame_area);

    // escrevendo em outro lugar (só mudar o ponto de inicio):
    char texto2[] = "* escrevo aqui";
    oled_draw_string(ssd, 16, 32, texto2, false);
    render_on_display(ssd, &frame_area);

  while(true) {
    sleep_ms(100000);  // Este aqui "hiberna" o processador, só "desperta" se ocorrer uma interrupção. Economiza mais energia
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
      "   BACKGROUND MENU   ",
      "---------------------",
      "                     ",
      "  ONE:               ",
      "                     ",
      "  TWO:               ",
      "                     ",
      "                     "
  };  
  
  for (int linha = 0; linha < 8; linha++) {
    coordenada_Y = linha * 8;
    oled_draw_string(ssd, 0, coordenada_Y, PG_INITIAL[linha], false);
  }
}