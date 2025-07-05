#include <stdio.h> 
#include "pico/stdlib.h" 
#include "FreeRTOS.h" 
#include "task.h" 
 
#define RGB_LED_G 11 
 
void blink_task(void *params) { 
    const uint LED_PIN = RGB_LED_G; 
    gpio_init(LED_PIN); 
    gpio_set_dir(LED_PIN, GPIO_OUT); 
 
    while (1) { 
        gpio_put(LED_PIN, 1); 
        vTaskDelay(pdMS_TO_TICKS(500)); 
        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500)); 
    } 
} 

int main() { 
    stdio_init_all(); 
    xTaskCreate(blink_task, "Blink", 256, NULL, 1, NULL); 
    /*
        "Blink" = Nome da tarefa, útil para identificar ou fazer debug.
        256 = Tamanho da pilha da tarefa, em palavras de 32 bits (1 palavra = 4 bytes).
        NULL = Parâmetros da tarefa, não utilizados neste caso.
        1 = Prioridade da tarefa, onde 1 é a prioridade mais baixa.
        NULL = Ponteiro para o identificador da tarefa, não utilizado neste caso.
    */
    vTaskStartScheduler(); 
    /*
        Esse comando inicia o escalonador do FreeRTOS, ou seja, a partir daqui o sistema começa 
            a gerenciar as tarefas criadas. Depois de chamado, o controle do programa passa a ser do RTOS.
        Tudo que vier depois desse comando não será mais executado diretamente — apenas por meio de tarefas.
    */
    while (true) {} 
} 
