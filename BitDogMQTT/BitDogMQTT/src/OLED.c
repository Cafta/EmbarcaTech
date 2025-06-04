#include <OLED.h>

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

// INÍCIO DO PROGRAMA
void oled_init() {
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
    const char texto[] = "   OLED INICIALIZADO  ";
    oled_draw_string(0, 16, texto, false);
    render_on_display(ssd, &frame_area);
}

void render() {
  // Renderiza o buffer no display
  render_on_display(ssd, &frame_area);
}

// Apaga o display totalmente
void clear_ssd1306_i2c() {
  memset(ssd, 0, ssd1306_buffer_length);
  render_on_display(ssd, &frame_area);
}

// Desenha um único caractere no display
void oled_draw_char(int16_t x, int16_t y, uint8_t character, bool invert) {
  int fb_idx = (y / 8) * 128 + x;
  
  for (int i = 0; i < 6; i++) {
    ssd[fb_idx++] = invert? ~FONT6x8[character - 0x20][i] : FONT6x8[character - 0x20][i];
  }
}

// Desenha uma string, chamando a função de desenhar caractere várias vezes
void oled_draw_string(int16_t x, int16_t y, const char *string, bool invert) {
  if (x > ssd1306_width - 6 || y > ssd1306_height - 8) {
      return;
  }

  x = (x == 0) ? 1: x;

  while (*string) {
      oled_draw_char(x, y, *string++, invert);
      x += 6;
  }
}

void print_oled(int16_t x, int16_t y, const char *string, bool invert) {
  oled_draw_string(x, y, string, invert);
  render_on_display(ssd, &frame_area); // Renderiza o buffer no display
}

void print_background() {
  uint8_t coordenada_Y = 0;
  clear_ssd1306_i2c();
  const char PG_INITIAL[8][22] = {  // 8 linhas com 21 caracteres + 1 para `\0`
      "SENSOR DE TEMPERATURA",
      "---------------------",
      "SEM Criptografia:    ",
      "                     ", // este muda
      "                     ",
      "COM Criptografia:    ",
      "                     ",  // este muda
      "                     "
  };  
  
  for (int linha = 0; linha < 8; linha++) {
    coordenada_Y = linha * 8;
    oled_draw_string(0, coordenada_Y, PG_INITIAL[linha], false);
  }

  render_on_display(ssd, &frame_area);
}