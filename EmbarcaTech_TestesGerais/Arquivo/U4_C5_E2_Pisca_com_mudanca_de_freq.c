/*  EMBARCATECH - 2024/25
    UNIDADE 4   CAP. 5
    ALUNO: CARLOS FERNANDO MATTOS DO AMARAL
    DESAFIO 1: 
    Elabore um programa para acionar um LED quando o botão A for pressionado 5 vezes, 
    tomando cuidado para registrar essa contagem. Quando o valor da contagem chegar a 5, 
    um LED deve piscar por 10 segundos, na frequência de 10 Hz.
    DESAFIO 2:
    Modifique o código da questão anterior, implementando o botão B para mudar a 
    frequência do LED para 1 Hz.
    (OBS: se quiser colocar mais frequencias, só aumentar o vetor blink_freq)
*/

#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdbool.h>
#include <stdio.h>

#define red 13
#define blue 12
#define green 11
#define btn_A 5 
#define btn_B 6


// Parametros de Configuração (enunciado):
#define turn_off_time_ms 10000
#define n_de_clicks 5
const int blink_freq[] = {10, 1}; // freq inicial(0); freq de mudança(1)

// funções e variáveis
void liga_pisca(bool liga, volatile int freq);  // Liga/desliga ou troca de frequencia o pisca-pisca
void led_inverter();  // inverte o led
void liga_led(bool liga); // Liga/desliga o led na cor branca
volatile int freq_ativa = 0;
volatile bool pisca_ligado = false; // registra o estado de funcionamento do pisca-pisca
volatile bool led_ligado = false; // registra o estatus do led
volatile bool botao_A_apertado = 1; // registra o estatus do botão (1 = solto, 0 = apertado)
volatile bool botao_B_apertado[] = {1, 1}; // {estado atual, ultimo estado}
volatile int btn_count = 0;  // a contagem do botão é feito dentro de um callback, por isso 'volatile'
volatile bool botao_solto = true; // funciona como uma "memória" de se o botão já foi solto, depois que foi apertado.
struct repeating_timer timer_blink;
struct repeating_timer timer_btn;

/**
 * @brief Piscador do Led 
 * Esta função de CallBack é acionada repetidamente na frequencia
 * definida por freq_piscagem
 * @param[in] *repeating_timer ponteiro do repetidor.
 * @param[out] booleano indica se a função manterá a repetição
 */
bool repeating_callback_pisca_led(struct repeating_timer *t) {
    led_inverter(); // inverte o led 
    return true;
}

/**
 * @brief Temporizador para desligar Led 
 */
int64_t turn_off_callback(alarm_id_t id, void *user_data) {
   liga_pisca(false, freq_ativa);
   return 0;
}

/**
 * @brief Leitura do Botão 
 * Avalia estatus do botão a cada 50ms, funcionando como um debouncing,
 * conta a quantidade de clicks, e aciona o pisca após o número de vezes
 * definido em n_de_clicks. Além disso aciona o timer para desligar o 
 * pisca após o tempo definido em turn_off_time_ms. 
 * @param[in] *repeating_timer ponteiro do timer de repetição
 */
bool repeating_callback(struct repeating_timer *t) {
    botao_A_apertado = gpio_get(btn_A); 
    botao_B_apertado[0] = gpio_get(btn_B);
    
    // TRATANDO BOTÃO A
    if (botao_A_apertado == 0 && pisca_ligado == false && botao_solto == true) { // só entra se o botão está apertado E o led desligado
        botao_solto = false; // apertou o botão mas não soltou ainda. Não é para entrar novamente neste if.
        printf("Botão pressionado %d vez%s.\n", btn_count + 1, (btn_count+1==1?"":"es"));
        if (++btn_count == n_de_clicks) {  // incrementa o count e compara com o numero desejado
            btn_count = 0; // reseta a contagem
            liga_pisca(true, freq_ativa);    // liga o led, de forma que não entra mais nesse if até ser desligado
            add_alarm_in_ms(turn_off_time_ms, turn_off_callback, NULL, false); // Cria o alarme para desligar o led
        }
    } else if (botao_A_apertado == 1) botao_solto = true; // Se o botão não está apertado, é porque ele está solto.
    
    // TRATANDO BOTÃO B
    if (botao_B_apertado[0] != botao_B_apertado[1]) {  // mudou o estado do botão
        if (botao_B_apertado[0] == 0) {  // acabou de apertar, e não de soltar
            freq_ativa++; // vai para próxima frequencia estipulada
            size_t len_blink_freq = sizeof(blink_freq)/sizeof(blink_freq[0]);  
            if (freq_ativa == len_blink_freq) freq_ativa = 0; // retorna ao 0 se acabou os índices do array 
            printf("Frequência definida para %dHz.\n", blink_freq[freq_ativa]);
            if (pisca_ligado == true) liga_pisca(true, freq_ativa); // modifica a freq com o pisca em andamento.
        }
        // printf("\n");
        botao_B_apertado[1] = botao_B_apertado[0]; // impede de entrar neste if até que o estado mude novamente.
    } 
    return true;
}

/**
 * @brief Função Principal
 */
int main() {
    stdio_init_all(); // permite a utilização do monitor serial
    
    // Iniciando GPIOs
    gpio_init(red);
    gpio_set_dir(red, GPIO_OUT);
    gpio_init(blue);
    gpio_set_dir(blue, GPIO_OUT);
    gpio_init(green);
    gpio_set_dir(green, GPIO_OUT);
    gpio_init(btn_A);
    gpio_set_dir(btn_A, GPIO_IN);
    gpio_pull_up(btn_A);
    gpio_init(btn_B);
    gpio_set_dir(btn_B, GPIO_IN);
    gpio_pull_up(btn_B);
  
    printf("Iniciou o programa.\n");
    printf("Testando o Led (este deve piscadar uma vez).\n");
    liga_led(true);
    sleep_ms(500);
    liga_led(false);

    add_repeating_timer_ms(50, repeating_callback, NULL, &timer_btn); // Aqui define o Timer de 50ms em repetição
                                                                  // O CallBack que verifica e faz a contagem do botão

    while(1) {
        sleep_ms(60000);
        printf("Deu uma volta de 1 min no loop. O repeating_timer está rolando solto sem interferir no loop.\n");
    }

    return 0;
}

/**
 * @brief Controla o Pisca-Pisca 
 * Esta função liga ou desliga o pisca, na frequencia definida no array blink_freq[]
 * Além disso troca a frequência se o pisca já está ligado em outra freq.
 * @param[in] Boolean Definindo se é para ligar ou desligar o pisca.
 * @param[in] int8_t Posição do array blink_freq que define a frequência de piscar.
 * @param[out] void
 */
void liga_pisca(bool liga, volatile int freq) {
    if (liga == true) {
        if (pisca_ligado == true) {
            // solicitaram ligar, mas já estava ligado... então deve ser para mudar de frequencia
            printf("Trocando a frequencia para %dHz.\n", blink_freq[freq]);
            if (cancel_repeating_timer(&timer_blink) == false) {
                printf("ERRO ao cancelar repeating_timer do timer_blink. (cod.155)\n");
            }
            freq_ativa = freq;
        } else {
            // solicitaram ligar e está desligado
            freq_ativa = freq;
            pisca_ligado = true;
            printf("Pisca ligado.\n");
        }
        if (!add_repeating_timer_ms(1000/blink_freq[freq_ativa], repeating_callback_pisca_led, NULL, &timer_blink)) {
            printf("ERRO ao adicionar repeating_timer do timer_blick. (cod.165)\n");
        }
    } else {
        if (!cancel_repeating_timer(&timer_blink)) {
            printf("ERRO ao cancelar repeating_timer do timer_blink. (cod.169)\n");
        }
        pisca_ligado = false;
        printf("Pisca-pisca desigado.\n");
        
    }
    liga_led(liga); // se desligar o pisca, com o led ligado, ele ficaria ligado sem essa linha de código
}

/**
 * @brief Inverte o estado do led
 */
void led_inverter() {
    liga_led(!led_ligado);
}

/**
 * @brief Liga/Desliga o led 
 * Esta função faz piscar o led na cor branca e registra se 
 * o pisca pisca está ligado ou desligado.
 * @param[in] Boolean definindo se é para ligar ou desligar o pisca.
 * @param[out] void
 */
void liga_led(bool liga) {
    led_ligado = liga;
    gpio_put(red, liga);
    gpio_put(blue, liga);
    gpio_put(green, liga);
}