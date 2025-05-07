/*  EMBARCATECH - 2024/25 
    UNIDADE 4   CAP. 5
    ALUNO: CARLOS FERNANDO MATTOS DO AMARAL
    
    EXEMPLO 0: *Código adaptado do e-book do EMBARCATECH
    Esse programa realiza uma leitura contínua da temperatura 
    interna do microcontrolador RP2040 e imprime o valor convertido em 
    graus Celsius no terminal, uma vez por segundo. É um exemplo simples e 
    eficaz de como utilizar o ADC para capturar dados analógicos e interpretá
    los em um sistema embarcado. 
    OBS: Não é nada preciso. Alguma estratégia precisa ser implementada para
    reduzir a variação de saída. Como média ou mediana.

    DESAFIO 6:
    Refaça o Programa Prático 00 presente no Ebook do Capítulo de ADC (Capítulo 8), mas implementando 
    no código a conversão da unidade de medida da temperatura de Celsius para Fahrenheit.
*/

#include <stdio.h>             // Biblioteca padrão para entrada e saída, utilizada para printf.
#include "pico/stdlib.h"       // Biblioteca padrão para funções básicas do Pico, como GPIO e temporização.
#include "hardware/adc.h"      // Biblioteca para controle do ADC (Conversor Analógico-Digital).


// Definições
#define ADC_TEMPERATURE_CHANNEL 4   // Canal ADC que corresponde ao sensor de temperatura interno
#define TAMANHO_DA_MEDIA 20  // Define o tamanho da amostra para cálculo da média

// Função para calcular a media
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

int main() {
    // Variáveis:
    float celsius;
    float fahrenheit;
    float celsius_med;
    float fahrenheit_med;

    // Inicializa a comunicação serial para permitir o uso de printf
    stdio_init_all();

    // Inicializa o módulo ADC do Raspberry Pi Pico
    adc_init();

    // Seleciona o canal 4 do ADC (sensor de temperatura interno)
    adc_set_temp_sensor_enabled(true);  // Habilita o sensor de temperatura interno
    adc_select_input(ADC_TEMPERATURE_CHANNEL);  // Seleciona o canal do sensor de temperatura

    sleep_ms(10000);  // para dar tempo de iniciar o monitor serial
    printf("Programa iniciado.\n");

    // Loop infinito para leitura contínua do valor de temperatura
    while (true) {
        // Lê o valor do ADC no canal selecionado (sensor de temperatura)
        uint16_t adc_value = adc_read();

        // Converte o valor do ADC para temperatura em graus Celsius
        adc_to_temperature(adc_value, &celsius, &fahrenheit, &celsius_med, &fahrenheit_med);

        // Imprime a temperatura na comunicação serial
        if (celsius_med != -1.0f) {
            printf("Temperatura registrada: %.2f °C => %.2f F ; Média: %.2f °C => %.2f F\n", celsius, fahrenheit, celsius_med, fahrenheit_med);
            sleep_ms(1000);
        } else {
            sleep_ms(10);
        }
        //sleep_ms(1000);
    }
    return 0;
}
