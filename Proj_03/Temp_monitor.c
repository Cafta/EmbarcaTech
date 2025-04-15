#include "pico/stdlib.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "inc/font6x8_com_pos_espaco.h"  // as funções oled_draw... dependem do tamanho dessas fontes
#include <string.h>  // precisa para o menset()
#include <stdio.h>
#include "hardware/adc.h"

// Definições Gerais
#define SSD1306_I2C_ADDR 0x3C
#define I2C_SDA 14
#define I2C_SCL 15
#define PAG_INICIAL_COD 1
#define PAG_PADRAO_COD 2
#define ADC_TEMPERATURE_CHANNEL 4   // Canal ADC que corresponde ao sensor de temperatura interno
#define TAMANHO_DA_MEDIA 30  // Define o tamanho da amostra para cálculo da média

// DEFINIÇÕES DE STRUCTS
struct render_area frame_area = {
  .start_column = 0,
  .end_column = ssd1306_width - 1,
  .start_page = 0,
  .end_page = ssd1306_n_pages - 1
};

// Buffer global para o display SSD1306 
uint8_t ssd[ssd1306_buffer_length];

// Definição de funções principais
void adc_to_temperature(uint16_t adc_value, float *celsius, float *fahrenheit, float *celsius_med, float *fahrenheit_med);
void print_values(uint16_t adc_value);

// Definição de funções acessórias:
void ssd1306_init(); // esta está em ssd1306_i2c.c (não é de minha autoria) 
void clear_ssd1306_i2c();
void OLED_draw_char(uint8_t *ssd, int16_t x, int16_t y, uint8_t character, bool invert) ;
void oled_draw_string(uint8_t *ssd, int16_t x, int16_t y, const char *string, bool invert);
float media(float ultimo_valor);
void temp_sensor_init();
void display_init();
void print_background();

// INÍCIO DO PROGRAMA
int main() {
    // Inicializando os periféricos
    stdio_init_all();
    display_init();   
    adc_init();
    temp_sensor_init();

    print_background();

  while(true) {
    // Lê o valor do ADC no canal selecionado (sensor de temperatura)
    uint16_t adc_value = adc_read();
    print_values(adc_value);
    sleep_ms(100);
  }

  return 0;
}

//  FUNÇÕES PRINCIPAIS

void print_values(uint16_t adc_value) {
    // Variáveis:
    float celsius;
    float fahrenheit;
    float celsius_med;
    float fahrenheit_med;
    char str_celsius[8];
    char str_fahrenheit[8];
    char str_dynamic[22];
    // Converte o valor do ADC para temperatura em graus Celsius
    adc_to_temperature(adc_value, &celsius, &fahrenheit, &celsius_med, &fahrenheit_med);
    // Transforma em string
    if (celsius_med == -1 || fahrenheit_med == -1) {
        snprintf(str_celsius, sizeof(str_celsius), "---oC");
        snprintf(str_fahrenheit, sizeof(str_fahrenheit), "---F");
    } else {
        snprintf(str_celsius, sizeof(str_celsius), "%.1foC", celsius_med);
        snprintf(str_fahrenheit, sizeof(str_fahrenheit), "%.0fF", fahrenheit_med);
    }
        snprintf(str_dynamic, sizeof(str_dynamic), "  ( %.1foC, %.0fF )", celsius, fahrenheit);
    // Printing values
    oled_draw_string(ssd, 11*6, 24, str_celsius, false);
    oled_draw_string(ssd, 14*6, 40, str_fahrenheit, false);
    oled_draw_string(ssd, 2, 56, str_dynamic, false);

    render_on_display(ssd, &frame_area);
}


//  FUNÇÕES ACESSÓRIAS

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

float media(float ultimo_valor) {
    static float ultimos_valores[TAMANHO_DA_MEDIA];
    static uint8_t indice = 0;
    static bool complete = false;
    float soma = 0.0f;

    ultimos_valores[indice] = ultimo_valor;
    if (++indice > TAMANHO_DA_MEDIA) {
        indice = 0;
        if (complete == false) complete = true;
    }
    if (complete) {
        for (uint8_t i = 0; i < TAMANHO_DA_MEDIA; i++) {
            soma = soma + ultimos_valores[i];
        }
        return soma/TAMANHO_DA_MEDIA;
    }
    return -1.0f;
}

// Função para converter o valor lido do ADC para temperatura em graus Celsius
void adc_to_temperature(uint16_t adc_value, float *celsius, float *fahrenheit, float *celsius_med, float *fahrenheit_med) {
    // Constantes fornecidas no datasheet do RP2040
    const float conversion_factor = 3.3f / (1 << 12);  // Conversão de 12 bits (0-4095) para 0-3.3V
    float voltage = adc_value * conversion_factor;     // Converte o valor ADC para tensão
    *celsius = 27.0f - (voltage - 0.706f) / 0.001721f;  // Equação fornecida para conversão em celsius
    *celsius_med = media(*celsius);
    *fahrenheit = (*celsius * 9.0f/5.0f) + 32.0f;       // Converte para Fahrenheit
    if (*celsius_med != -1.0f) {
        *fahrenheit_med = (*celsius_med * 9.0f/5.0f) + 32.0f;
    } else {
        *fahrenheit_med = -1;
    }
}

void temp_sensor_init() {
    // Seleciona o canal 4 do ADC (sensor de temperatura interno)
    adc_set_temp_sensor_enabled(true);  // Habilita o sensor de temperatura interno
    adc_select_input(ADC_TEMPERATURE_CHANNEL);  // Seleciona o canal do sensor de temperatura
}

void display_init() {
    // Iniciando o Display:
    i2c_init(i2c1, ssd1306_i2c_clock * 1000); // Inicializa I2C a 400MHz (definido em ssd1306_ic2.h)
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // SDA
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // SCL
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();  // Comandos de Inicialização do ssd1306 definido em ssd1306_i2c.c
    calculate_render_area_buffer_length(&frame_area); // função em ssd1306_i2c.c
    clear_ssd1306_i2c();   // zera o display inteiro
}

void print_background() {
    uint8_t coordenada_Y = 0;
    clear_ssd1306_i2c();
    const char PG_INITIAL[8][22] = {  // 8 linhas com 21 caracteres + 1 para `\0`
        "   CPU TEMPERATURE   ",
        "---------------------",
        "                     ",
        "  Celsius:           ",
        "                     ",
        "  Fahrenheit:        ",
        "                     ",
        "                     "
    };  
    
    for (int linha = 0; linha < 8; linha++) {
    coordenada_Y = linha * 8;
    oled_draw_string(ssd, 0, coordenada_Y, PG_INITIAL[linha], false);
    }
}
