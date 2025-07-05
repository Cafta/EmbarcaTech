#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h" 
#include "task.h" 
#include "hardware/pwm.h"

/*
    No arquivo main.c, implemente: 
    ● A tarefa do LED RGB (troca de cores a cada 500ms) 
    ● A tarefa do buzzer (beep curto a cada 1 segundo) 
    ● A tarefa dos botões (polling a cada 100ms) 
*/

// Variável global
bool Bpressed = false; // Variável para armazenar o estado do botão

// ● A tarefa do LED RGB (troca de cores a cada 500ms) 
void blink_task(void *params) { 
    #define RGB_LED_G 11 
    #define RGB_LED_R 12 
    #define RGB_LED_B 13
    bool led[] = {1, 0, 0}; // R, G, B

    gpio_init(RGB_LED_G);
    gpio_init(RGB_LED_R);
    gpio_init(RGB_LED_B); 
    gpio_set_dir(RGB_LED_B, GPIO_OUT); 
    gpio_set_dir(RGB_LED_R, GPIO_OUT);
    gpio_set_dir(RGB_LED_G, GPIO_OUT);
 
    while (1) { 
        // troca de cor
        if (led[0] == 1) {
            led[0] = 0;
            led[1] = 1;  
        } else if (led[1] == 1) {
            led[1] = 0;
            led[2] = 1;  
        } else {
            led[2] = 0;
            led[0] = 1;  
        }
        // atualiza o LED RGB
        gpio_put(RGB_LED_R, led[0]);
        gpio_put(RGB_LED_G, led[1]);
        gpio_put(RGB_LED_B, led[2]);
        // aguarda 500ms
        vTaskDelay(pdMS_TO_TICKS(500)); 
    } 
} 

// ● A tarefa do buzzer (beep curto a cada 1 segundo) 
void buzzer_task(void *params) {
    #define BUZZER_PIN 21
    // gpio_init(BUZZER_PIN);
    // gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_set_function(BUZZER_PIN GPIO_FUNC_PWM);     
    pwm_set_clkdiv(gpio_buzzer_slice, 100); // define o divisor do clock do RP2040 (125MHz / 100 = 1250KHz)
    pwm_set_wrap(gpio_buzzer_slice, 8000);  // "empacota" o ciclo em 8000 clocks (1250KHz / 8000 = 156Hz)
    pwm_set_gpio_level(buzzer, 0);  // mantém High por 0 clocks => cycle em 0% => "desliga o buzzer" 
    pwm_set_enabled(gpio_buzzer_slice, true);  // inicia o canal (slice)

    while (1) {
        gpio_put(BUZZER_PIN, 1); // ativa o buzzer
        vTaskDelay(pdMS_TO_TICKS(100)); // aguarda 100ms
        gpio_put(BUZZER_PIN, 0); // desativa o buzzer
        vTaskDelay(pdMS_TO_TICKS(900)); // aguarda 900ms
    }
}

int main()
{
    stdio_init_all();

    // ● A tarefa do LED RGB (troca de cores a cada 500ms) 
    xTaskCreate(blink_task, "Blink", 256, NULL, 1, NULL); 
    // ● A tarefa do buzzer (beep curto a cada 1 segundo) 

    // ● A tarefa dos botões (polling a cada 100ms)
    // Inicializa o RTOS
    vTaskStartScheduler();
    // Se o RTOS não iniciar, entra em loop infinito
    while (true) { // nunca chega aqui
        printf("Erro ao iniciar o RTOS\n");
        printf("Reinicie o programa\n");
        sleep_ms(1000);
    }
}
