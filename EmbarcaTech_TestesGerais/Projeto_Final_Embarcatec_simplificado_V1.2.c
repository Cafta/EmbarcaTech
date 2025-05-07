/*  EMBARCATECH - 2024/25
    UNIDADE 7   PROJETO FINAL DA PRIMEIRA FASE
    ALUNO: CARLOS FERNANDO MATTOS DO AMARAL
    DESCRIÇÃO:
  Este projeto tem por objetivo desenvolver um dispositivo de auxílio ao monitoramento dos
  tempos preconizados pela AHA durante a atuação da equipe de emergência na RCP com indicação 
  de tratamento sugerido pela ACLS.

    FUNCIONALIDADES:
  (1)	Controle do tempo de dois minutos para a troca do responsável pela massagem cardíaca;
  (2)	Controle do tempo de dez segundos de pausa entre as massagens;
  (3)	Cálculo em tempo real da FCT;
  (4)	Incluir um metrônomo, com indicação visual e auditiva, de referência para frequência correta das compressões torácicas;
  (5)	Sugestão de conduta em função do ritmo cardíaco apresentado durante as verificações, de acordo com os algoritmos do ACLS.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include "hardware/clocks.h"
#include "inc/font6x8_com_pos_espaco.h"
#include "inc/msg.h"

// Definições Gerais
#define SSD1306_I2C_ADDR 0x3C
#define I2C_SDA 14
#define I2C_SCL 15
#define btn_A 5
#define btn_B 6
#define PAG_INICIAL_COD 1
#define PAG_PADRAO_COD 2
uint8_t active_page = PAG_INICIAL_COD;
#define DEBOUNCING_DELAY_MS 80
#define SW_PIN 22
#define VRX_PIN 27
#define VRY_PIN 26
#define VRX 1
#define VRY 0
#define MATRIX_PIN 7
#define NUM_LEDS 25 // Matrix 5x5 = 25 leds
#define NEUTRO 0  // posição do jostick
#define SUBIR 1   // posição do jostick
#define DESCER 2  // posição do jostick
#define led_R 13
#define led_G 11
#define led_B 12
#define buzzer 21
#define buzzer_freq 1500  // entre 90Hz e 3kHz, aprox. Pois o divisor tem só 8 bits, tem que ficar
#define buzzer_top 8000  // entre 1 e 255. Fórmula: divisor = sys_clk / ((buzzer_freq + 1) * buzzer_top)

// Definições de constantes do MENU:
typedef enum {
  ACTIVE_MENU,  // às vezes tem que desenhar o mesmo menu, indiferente de qual seja.
  MENU_RITMO_CARDIACO,
  MENU_PULSO,
  MENU_AGUARDA_CONDUTA,
  MENU_DESLIGADO,
  ICM_METRONOME
} menus;

// Definições de constantes de condutas
typedef enum {
  DESFIBRILAR,
  MASSAGEAR,
  CARDIOVERTER
} condutas;

// Definições de constantes das respostas dos menus:
typedef enum {
  REGULAR = 0,
  FV,
  TVMONO,
  TVPOLI,
  FEITO = 0,
  IGNORADO,
  COM_PULSO = 0,
  SEM_PULSO,
  LIGAR = 0,
  DESLIGAR,
  OFF = 0,
  ON
} respostas_menu; 

// DEFINIÇÕES DE STRUCTS
struct render_area frame_area = {
  .start_column = 0,
  .end_column = ssd1306_width - 1,
  .start_page = 0,
  .end_page = ssd1306_n_pages - 1
};

struct menu_config {
  menus codigo;
  uint8_t n_opcoes;
  char opcoes[5][11];
  respostas_menu select;
};

// Variaveis Globais
PIO pio = pio0;
uint sm;
volatile bool iniciada_RCP = false; // quando inicia ele precisa resetar todos os tempos.
uint64_t total_compress_time = 0;
uint64_t total_stall_time = 0;
volatile bool compress_state[] = {0, 0}; // indica se está realizando compressão torácica {estado atual, último estado} 
volatile absolute_time_t RCP_start_time = 0;  // a cada retomada da compressão
volatile absolute_time_t compress_start_time = 0;  // a cada retomada da compressão
volatile absolute_time_t stall_start_time = 0;     // a cada parada de compressão
volatile condutas conduta_sugerida;   // nem sempre volta para o mesmo menu.
respostas_menu previous_answer;  // só é usado se for para o MENU_PULSO, porque faz diferença qual o rítmo
bool led_aceso = false;
bool flip_flop = false;
int16_t buzzer_vol = buzzer_top * 0.3; // volume do buzzer
uint8_t joystick_position[] = {NEUTRO, NEUTRO}; // {ATUAL, ANTERIOR}
struct menu_config ritmos_cardiacos;
struct menu_config pulso;
struct menu_config aguarda_conduta;
struct menu_config desligado;
struct menu_config icm_metronome;
struct menu_config *menu_active;
static absolute_time_t last_callback_call_time = 0; 
struct repeating_timer general_timer;
struct repeating_timer clock_timer;
struct repeating_timer alert_timer;
struct repeating_timer metronome_timer;
uint8_t alert_counter[3]; 

// Buffer global para o display SSD1306 
uint8_t ssd[ssd1306_buffer_length];

// Definição de funções principais
void print_page(uint8_t codigo_pg);
void print_menu(uint8_t cod);
void print_conduta(const char msg[][11]);
void button_callback(uint gpio, uint32_t events);
bool repeating_callback(struct repeating_timer *t);
bool display_time_update_callback(struct repeating_timer *t);
void make_decision();
void RCE();  // Retorno a Circulação Expontânea (Final Feliz)

// Definição de funções acessórias:
void ssd1306_init(); // esta está em ssd1306_i2c.c (não é de minha autoria) 
void helper_init_all();
void clear_ssd1306_i2c();
uint8_t read_joystick();
void set_light(bool valor);
void set_buzzer(bool valor);
void OLED_draw_char(uint8_t *ssd, int16_t x, int16_t y, uint8_t character, bool invert) ;
void oled_draw_string(uint8_t *ssd, int16_t x, int16_t y, const char *string, bool invert);
void set_menus();
int64_t metronome_off_callback();
bool heart_beat_frequency_callback();
void set_buzzer_frequency(uint16_t freq);
void set_metronome(bool status);
void ws2812_send(uint32_t color);
void clear_ws2812();
void set_ws2812(uint32_t colors[NUM_LEDS]);
void set_heart(bool status);
int64_t cancel_alarm_callback();
bool alert_callback(repeating_timer_t *t);
void alert(uint8_t n_chamadas, uint16_t frequencia_ms, bool final_beep);

// INÍCIO DO PROGRAMA
int main() {
  stdio_init_all();
  helper_init_all();  // Inicializa todos os acessórios

  // Setando as Interrupções:
  gpio_set_irq_enabled_with_callback(btn_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &button_callback);
  gpio_set_irq_enabled_with_callback(btn_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &button_callback);
  gpio_set_irq_enabled_with_callback(SW_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &button_callback); // Btn do Jostick
  add_repeating_timer_ms(100, repeating_callback, NULL, &general_timer);
  add_repeating_timer_ms(1025, display_time_update_callback, NULL, &clock_timer);

  print_page(PAG_INICIAL_COD);   // Exibe Pagina Inicial

  // teste do buzzer (apagar)
  set_buzzer(true);
  set_heart(true);
  sleep_ms(100);
  set_buzzer(false);
  clear_ws2812();

  // Setando os temporizadores

  // Criando os menus:
  set_menus();

  while(true) {
    // tight_loop_contents();  // Este aqui é para se eu precisar de uma resposta a interrupção bem imediata (consome mais energia)
    sleep_ms(10000);  // Este aqui "hiberna" o processador, só "desperta" se ocorrer uma interrupção. Economiza mais energia
  }

  return 0;
}

// ##########       FUNÇÕES PRINCIPAIS      ###########

// Cria a página de pano de fundo
void print_page(uint8_t codigo_pg) {
  clear_ssd1306_i2c();
  uint8_t coordenada_Y = 0;
  if (codigo_pg == PAG_INICIAL_COD) {  
    active_page = PAG_INICIAL_COD;
    iniciada_RCP = false;
    for (int linha = 0; linha < 8; linha++) {
      coordenada_Y = linha * 8;
      oled_draw_string(ssd, 0, coordenada_Y, PG_INITIAL[linha], false);
    }
  } 
  else if (codigo_pg == PAG_PADRAO_COD) {  
    active_page = PAG_PADRAO_COD;
    if (iniciada_RCP == false) { // tá iniciando a RCP agora
      // iniciada_RCP = true;
      //compress_start_time = get_absolute_time();  
      RCP_start_time = get_absolute_time();  // reseta os tempos
      total_compress_time = 0;
      total_stall_time = 0;
    }
    int x = 0;
    for (int linha = 0; linha < 8; linha++) {
      coordenada_Y = linha * 8;
      oled_draw_string(ssd, 0, coordenada_Y, PG_DEFAULT[linha], false);
    }
    ssd1306_draw_line(ssd, 0, 9, 127, 9, true);
    ssd1306_draw_line(ssd, 64, 9, 64, 63, true);

    print_conduta(MSG_MASSAGEM);
    print_menu(MENU_RITMO_CARDIACO);
  }
  render_on_display(ssd, &frame_area);
}

// Menu do lado direito do OLED
void print_menu(menus cod) {
  uint8_t coordenada_y = 15;   // localiza na tela onde vai
  uint8_t coordenada_x = 67;   // começar a desenhar
  bool inverso = false;
  char icm_menu[6][11];
  uint8_t linha_final = 6;

  if (cod == MENU_RITMO_CARDIACO) menu_active = &ritmos_cardiacos;
  else if (cod == MENU_PULSO) menu_active = &pulso;
  else if (cod == MENU_AGUARDA_CONDUTA) menu_active = &aguarda_conduta;
  else if (cod == MENU_DESLIGADO) menu_active = &desligado;
  else if (cod == ICM_METRONOME || menu_active->codigo == ICM_METRONOME) {  // menus ICM contém título e só aparecem se o menu atual for o MENU_DESLIGADO
    menu_active = &icm_metronome;
    coordenada_y += 8; 
    oled_draw_string(ssd, coordenada_x, coordenada_y, "METRONOMO ", false);
    coordenada_y += 8; 
    oled_draw_string(ssd, coordenada_x, coordenada_y, "          ", false);
    linha_final -= 2;
  } 
  // se não for nenhum desses, mantém o mesmo menu.

  for (int linha = 0; linha < linha_final; linha++) {
    coordenada_y += 8;
    inverso = (linha == menu_active->select)? true : false;
    if (linha < menu_active->n_opcoes) {
      oled_draw_string(ssd, coordenada_x, coordenada_y, menu_active->opcoes[linha], inverso);
    } else {
      // imprime linha em branco
      char linha_em_branco[] = "          ";
      oled_draw_string(ssd, coordenada_x, coordenada_y, linha_em_branco, false);
    }
  }
  render_on_display(ssd, &frame_area);
}

// Mensagem do lado esquerdo do OLED
void print_conduta(const char msg[][11]) {
  uint8_t coordenada_y = 18; 
  for (int linha = 0; linha < 6; linha++) {
    oled_draw_string(ssd, 0, coordenada_y, msg[linha], false);
    coordenada_y += 8;
  }
} 

// Define a ação no acionamento dos botões
void button_callback(uint gpio, uint32_t events) {
  static absolute_time_t last_click_time = 0;

  // DEBOUNCING
  absolute_time_t now = get_absolute_time();
  int64_t spent_time = absolute_time_diff_us(last_click_time, now) / 1000;

  if (spent_time > DEBOUNCING_DELAY_MS) {  // Proteção Debouncing
    if (events & GPIO_IRQ_EDGE_RISE) {  // EDGE_RISE indica que o botão foi precionado e não liberado
      // Alarm estiver ligado, desliga ao pressionar qualquer botão.
      if (alert_timer.alarm_id != 0) {
        cancel_repeating_timer(&alert_timer);
        set_buzzer(OFF);
        set_light(OFF);
      } 

      // FUNÇÃO DO BOTÃO A
      if (gpio == btn_A) {
        if (active_page == PAG_INICIAL_COD) {
          print_page(PAG_PADRAO_COD);
          print_conduta(MSG_INITIAL); 
          print_menu(MENU_RITMO_CARDIACO);
        } else {  // na segunda vez que aperta o A depois da tela inicial
          compress_state[0] = !compress_state[0]; // Lida com isso em repeating_callback --> timer_ms_update()
        }
        if (!iniciada_RCP) {
          iniciada_RCP = true;
          stall_start_time = get_absolute_time();
        } else {
          set_metronome(OFF); // se metrônomo estiver ligado, desliga
        }
      }
      
      // FUNÇÃO DO BOTÃO B (ou do Jostick)
      else if (gpio == btn_B || gpio == SW_PIN) {
        if (active_page == PAG_PADRAO_COD) {
          //printf("BTN ação Acionado --> ");
          //printf("menu: %d; selecionado: %d \n", menu_active->codigo, menu_active->select);
          make_decision();
        }
      } 
    }

    last_click_time = get_absolute_time();  // Reseta a contagem do bouncing
  }
}

// Callback de atualização lenta (1s):
// (1) Controla o tempo total e a FCT 
// (2) Chama considerar_medicacao() para ver se tem drogas para fazer
// (3) Abre o menu do metrônomo
bool display_time_update_callback(struct repeating_timer *t) {
  if (!iniciada_RCP) return true;

  // Abre menu do metrônomo se o menu ativo for MENU_DESLIGADO
  if (menu_active->codigo == MENU_DESLIGADO) print_menu(ICM_METRONOME);

  // Avalia medicações
  //considerar_medicacao(); 

  // Atualiza hora e FCT
  uint8_t coordenada_x = 0, coordenada_y = 0;
  uint16_t total_minutos = absolute_time_diff_us(RCP_start_time, get_absolute_time())/60000000;
  uint16_t horas = total_minutos / 60;
  uint16_t min = total_minutos % 60;
  char relogio_string[7];
  snprintf(relogio_string, 7, "%02dh%02dm", horas, min);
  oled_draw_string(ssd, coordenada_x, coordenada_y, relogio_string, false);
  if (total_stall_time != 0) {
    coordenada_x = 109; 
    uint64_t fct = (100ULL * total_compress_time / (total_compress_time + total_stall_time)) ;
    char fct_string[4];
    sprintf(fct_string, "%3u", fct);
    oled_draw_string(ssd, coordenada_x, coordenada_y, fct_string, false);
  }
  render_on_display(ssd, &frame_area);
  
  return true;
}

// (1) Atualiza tempo de compressão e stall, e o cálculo do fct. 
// (2) Mostra na tela o tempo de compressão sendo contado, e o stall também, 
// mas não a fct | isso é feito no display_time_update_callback() (por causa do tempo mais longo)
void timer_ms_update(absolute_time_t now) {
  // Exitem 2 estados possíveis. Compressão ou não compressão (stall)
  if (!iniciada_RCP) return;
  if (compress_state[0]) {  // estado atual, está fazendo a massagem cardíaca. Quem define é o botão A (button_callback)
    if (compress_state[0] != compress_state[1]) {  // acabou de mudar de estado
      compress_state[1] = compress_state[0];
      compress_start_time = now;
      print_menu(MENU_DESLIGADO);
      print_conduta(MSG_MASSAGEANDO);
    }
    // Estando comprimindo adiciona o tempo ao tempo de compressão
    total_compress_time += absolute_time_diff_us(last_callback_call_time, now)/1000;
    //printf("feedback304: comp_time: %llu, stall_time: %llu \n", total_compress_time, total_stall_time);
    // Primeiro vamos trocar a mensagem de acordo com o tempo de compressão:
    uint64_t compress_time = absolute_time_diff_us(compress_start_time, now)/1000;
    char msg[11];
    // agora vamos verificar se já está perto dos 2'
    if (compress_time/60000 >= 2) {  // já passou de 2 min.
      print_conduta(MSG_TROCAR_MASSAGISTA);      // Colocar o alerta de tempo
    } else if (compress_time >= 115000) {  // 5000ms para dar 2 min.
      set_buzzer_frequency(1500);
      if (alert_timer.alarm_id == 0) alert(5, 1000, false);
    } else if (compress_time/60 > 1500) { // já passou 1min e meio
      print_conduta(MSG_PREPARA_TROCA);
    }
    // Agora vamos imprimir o tempo desta compressão (não o total):
    int8_t min = compress_time/(60*1000);
    int8_t seg = (compress_time%(60*1000))/1000;
    snprintf(msg, 11, "  %02dm%02ds  ", min, seg);
    oled_draw_string(ssd, 0, 56, msg, false);

    // Renderiza tudo no OLED
    render_on_display(ssd, &frame_area);
  } else {
    // Se não está comprimindo adiciona o tempo ao stall
    if (compress_state[0] != compress_state[1]) {  // mudou de estado.
      compress_state[1] = compress_state[0];
      stall_start_time = now;
      print_conduta(MSG_CHECAR_RITMO);
      print_menu(MENU_RITMO_CARDIACO);
    }
    total_stall_time += absolute_time_diff_us(last_callback_call_time, now)/1000;

    // Agora vamos imprimir o tempo de STALL (não o total):
    uint64_t stall_time = absolute_time_diff_us(stall_start_time, now)/1000;
    char msg[11];
    int8_t min = stall_time/(60*1000);
    int8_t seg = (stall_time%(60*1000))/1000;
    snprintf(msg, 11, "stop:%dm%02ds", min, seg);
    oled_draw_string(ssd, 0, 56, msg, false);

    // Colocar o alerta de tempo
    if (stall_time >= 5000 && stall_time < 6000) {  // "< 6000" evita que continue apitando depois dos 5s de alarme.
      set_buzzer_frequency(1500);
      if (alert_timer.alarm_id == 0) alert(5, 1000, true);
    }

    // Renderiza tudo no OLED
    render_on_display(ssd, &frame_area);
  }
  last_callback_call_time = get_absolute_time();
};

// (1) Atualização de 50ms. Chama o timer_ms_update().
// (2) Verifica a posição do Jostick mudando a seleção do Menu ativo.
bool repeating_callback(struct repeating_timer *t) {
  absolute_time_t now = get_absolute_time();  // esse se apaga

  timer_ms_update(now);

  // (3) Verificando a possição do Jostick e mudano a seleção do Menu Ativo
  joystick_position[0] = read_joystick();
  if (joystick_position[0] != joystick_position[1]) {
    // mudou a posição do jostick
    joystick_position[1] = joystick_position[0];

    // mudando o menu
    char pos[7] = "NEUTRO";
    if (joystick_position[0] == SUBIR) {
      if (menu_active->select > 0) {
        menu_active->select = menu_active->select - 1;
      } else {
        menu_active->select = menu_active->n_opcoes - 1;
      }
      strcpy(pos, "SUBIR ");
    } 
    else if (joystick_position[0] == DESCER) {
      if (menu_active->select == menu_active->n_opcoes - 1) {
        menu_active->select = 0;
      } else {
        menu_active->select = menu_active->select + 1;
      }
      strcpy(pos, "DESCER");
    }
    print_menu(ACTIVE_MENU);
  };
  return true;
}

void make_decision() {;
  respostas_menu selecionado = menu_active->select;

  //  MENU_RITMO_CARDIACO
  if (menu_active->codigo == MENU_RITMO_CARDIACO) {
    // Toma as decisões em relação ao algoritmo do ACLS
    if (selecionado == FV || selecionado == TVPOLI) {
      print_conduta(MSG_DESFIBRILAR);
      conduta_sugerida = DESFIBRILAR;
      print_menu(MENU_AGUARDA_CONDUTA);

    } else if (selecionado == TVMONO) {
      print_conduta(MSG_CHECAR_PULSO);
      previous_answer = TVMONO;
      print_menu(MENU_PULSO);
    }

    else if (selecionado == REGULAR) {
      print_conduta(MSG_CHECAR_PULSO);
      previous_answer = REGULAR;
      print_menu(MENU_PULSO);
    }
  }
  // MENU ICM_METRONOME
  else if (menu_active->codigo == ICM_METRONOME) {
    if(selecionado == LIGAR) {
      //printf("metronome_timer.alarm_id = %d\n" , metronome_timer.alarm_id);
      set_metronome(ON);
      if (metronome_timer.alarm_id == 0) { // isso evita que se ligue mais de uma vez enquanto ainda está ativo
        set_buzzer_frequency(500);
        add_repeating_timer_ms(545/2, heart_beat_frequency_callback, NULL, &metronome_timer);  // 545ms = freq. aproximada de 110 compressões por min
      }
        // add_alarm_in_ms(3000, metronome_off_callback, NULL, false);
    } else {
      set_metronome(OFF);
      cancel_repeating_timer(&metronome_timer);
      set_buzzer(false);
    }
  }  
  // MENU_AGUARDA_CONDUTA
  else if (menu_active->codigo == MENU_AGUARDA_CONDUTA) {
    // CONDUTA ERA: DESFIBRILAR
    if (conduta_sugerida == DESFIBRILAR) {
      if (menu_active->select == FEITO || menu_active->select == IGNORADO) {
          print_conduta(MSG_MASSAGEM); 
          conduta_sugerida = MASSAGEAR;
          print_menu(MENU_AGUARDA_CONDUTA);
      }
    }

    // CONDUTA ERA: CARDIOVERTER
    else if (conduta_sugerida == CARDIOVERTER) {
      //como se tivesse acionando o botão A
        if (menu_active->select == FEITO) {
          print_conduta(MSG_CHECAR_RITMO);
          print_menu(MENU_RITMO_CARDIACO);
        } // se select for IGNORAR indicar MASSAGEM novamente.

        else if (compress_state[0] == 1) {
          print_conduta(MSG_MASSAGEANDO);
          print_menu(MENU_DESLIGADO);
        }

        else {
          print_conduta(MSG_MASSAGEM);
          print_menu(MENU_AGUARDA_CONDUTA);
        }
      }

    // CONDUTA ERA: MASSAGEAR
    else if (conduta_sugerida == MASSAGEAR) {
    //como se tivesse acionando o botão A
      if (menu_active->select == FEITO) {
        compress_state[0] = !compress_state[0];
      } // se select for IGNORAR não faz nada, apenas muda o menu
      if (compress_state[0] == 1) print_conduta(MSG_MASSAGEANDO);
      print_menu(MENU_DESLIGADO);    
    }
  }
    // MENU_PULSO
  else if (menu_active->codigo == MENU_PULSO) {
    if (menu_active->select == COM_PULSO) {
      if (previous_answer == TVMONO) {
        print_conduta(MSG_CARDIOVERTER);
        conduta_sugerida = CARDIOVERTER;
        print_menu(MENU_AGUARDA_CONDUTA);
      } else if (previous_answer == REGULAR) {
        RCE();
      }
    } else { // Ritmo regular sem pulso ou TVMONO sem pulso
      if (previous_answer == TVMONO) {
        print_conduta(MSG_DESFIBRILAR);
        conduta_sugerida = DESFIBRILAR;
        print_menu(MENU_AGUARDA_CONDUTA);
      } else {  // só pode ser REGULAR Sem Pulso (AESP - Atividade Elétrica Sem Pulso)
        print_conduta(MSG_MASSAGEM);
        conduta_sugerida = MASSAGEAR;
        print_menu(MENU_AGUARDA_CONDUTA);
      }
    }
  }
}

// ##########       FUNÇÕES ACESSÓRIAS      ###########

// Inicia todos os periféricos utilizados no RCP Helper
void helper_init_all() {
  // Iniciando o Display:
  i2c_init(i2c1, ssd1306_i2c_clock * 1000); // Inicializa I2C a 400MHz (definido em ssd1306_ic2.h)
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // SDA
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // SCL
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);
  ssd1306_init();  // Comandos de Inicialização do ssd1306 definido em ssd1306_i2c.c
  calculate_render_area_buffer_length(&frame_area); // função em ssd1306_i2c.c
  clear_ssd1306_i2c();   // zera o display inteiro
  
  // Setando os botões
  gpio_init(btn_A);
  gpio_set_dir(btn_A, GPIO_IN);
  gpio_pull_up(btn_A);
  gpio_init(btn_B);
  gpio_set_dir(btn_B, GPIO_IN);
  gpio_pull_up(btn_B);
  gpio_init(SW_PIN);  // botão do jostick
  gpio_set_dir(SW_PIN, GPIO_IN);
  gpio_pull_up(SW_PIN);

  // Setando o Jostick
  adc_init();
  adc_gpio_init(VRX_PIN);
  adc_gpio_init(VRY_PIN);

  // Setando o Led
  gpio_init(led_R); gpio_init(led_G); gpio_init(led_B);
  gpio_set_dir(led_R, GPIO_OUT); gpio_set_dir(led_G, GPIO_OUT); gpio_set_dir(led_B, GPIO_OUT); 

  // Setando o Buzzer
  gpio_set_function(buzzer, GPIO_FUNC_PWM);  // define a porta do buzzer como saída pwm
  uint gpio_buzzer_slice = pwm_gpio_to_slice_num(buzzer);  // descobre qual o Canal (slice) de frequencia desta porta
     // agora vamos mudar a frequência deste canal (slice)
     /*  PROBLEMA COM ESSA FREQUÊNCIA !!! 
       A frequência de audição humana, varia de 20Hz até 20Mhz. 
       O problema ocorre, porque o RP2040 tem um divisor de 8bits inteiros e 8bits fracionários. O que dá um máximo valor
       de 256. Ou seja, o divisor do clock tem que ficar entre 1 e 255. 
       Inicialmente coloquei um top de 4095, mas com isso o cálculo do divisor em 90Hz ficaria em 339, o que 
       passaria do limite de 255. 
       Assim, troquei o top para 8000 que fica com o divisor de 174, dentro do aceitável. 
       com frequencia de 3kHz, o divisor ficaria em 5. Também válido.
     */
     float divisor = clock_get_hz(clk_sys)/(buzzer_freq*(buzzer_top+1)); 
     // via das dúvidas, vamos manter uma segurança:
        if (divisor > 256) divisor = 256;
        if (divisor <= 1) divisor = 1; 
     pwm_set_clkdiv(gpio_buzzer_slice, divisor); 
     pwm_set_wrap(gpio_buzzer_slice, buzzer_top); 
     pwm_set_gpio_level(buzzer, 0);  // "desliga o buzzer"
     pwm_set_enabled(gpio_buzzer_slice, true);  // inicia o canal (slice)

  // Configuração do PIO para controlar matriz de LEDs
  uint offset = pio_add_program(pio, &ws2812_program);
  sm = pio_claim_unused_sm(pio, true);
  ws2812_program_init(pio, sm, offset, MATRIX_PIN, 800000, false);
}

// Apaga o display totalmente
void clear_ssd1306_i2c() {
  memset(ssd, 0, ssd1306_buffer_length);
  render_on_display(ssd, &frame_area);
}

// Retorna a orientação do jostick (SUBIR, DESCER, NEUTRO)
uint8_t read_joystick() {
  // Ler Valor
  adc_select_input(VRY);  // GP26 = ADC0  
  uint16_t vry_value = adc_read();

  // Definir se é para subir ou descer
  if (vry_value > 3048) {  
    return SUBIR;
  } else if (vry_value < 1048) {
    return DESCER;
  } else if (vry_value > 1100 && vry_value < 3000) {  // histerese
    return NEUTRO;
  } 
}

// liga ou deliga o led, em função do valor booleano recebido
void set_light(bool valor) {
  led_aceso = valor;
  gpio_put(led_R, valor);
  gpio_put(led_G, valor);
  gpio_put(led_B, valor);
}

// liga/desliga o som do buzzer em função do valor booleano recebido
void set_buzzer(bool valor) {
  if (valor) {
    pwm_set_gpio_level(buzzer, buzzer_vol); // "volume"
  } else {
    pwm_set_gpio_level(buzzer, 0); // zera o volume
  }
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

void set_menus() {
        // MENU RITMOS CARDIACOS
  ritmos_cardiacos.codigo = MENU_RITMO_CARDIACO;
  ritmos_cardiacos.n_opcoes = 4;
  ritmos_cardiacos.select = FV;
  strcpy(ritmos_cardiacos.opcoes[REGULAR], "R.Regular ");
  strcpy(ritmos_cardiacos.opcoes[FV], "   FV     ");
  strcpy(ritmos_cardiacos.opcoes[TVMONO], "TV monomor");
  strcpy(ritmos_cardiacos.opcoes[TVPOLI], "TV polimor");

        // MENU_PULSO
  pulso.codigo = MENU_PULSO;
  pulso.n_opcoes = 2;
  pulso.select = COM_PULSO;
  strcpy(pulso.opcoes[SEM_PULSO], "SEM PULSO ");
  strcpy(pulso.opcoes[COM_PULSO], "COM PULSO ");

        // MENU_AGUARDA_CONDUTA
  aguarda_conduta.codigo = MENU_AGUARDA_CONDUTA;
  aguarda_conduta.n_opcoes = 2;
  aguarda_conduta.select = FEITO;
  strcpy(aguarda_conduta.opcoes[FEITO], " FEITO    ");
  strcpy(aguarda_conduta.opcoes[IGNORADO], " IGNORAR  ");

       // MENU_DESLIGADO
  desligado.codigo = MENU_DESLIGADO;
  desligado.n_opcoes = 0;
  desligado.select = 0;

// *** INTRA COMPRESSION MENUS ***
        // Estes menus ocorrem apenas no período que está massageando e 
        // não interferem com os outros menus.
        // Além disso tem prioridade inferior, de forma que só aparecem
        // Se não tiver outro menu ligado.
  icm_metronome.codigo = ICM_METRONOME;
  icm_metronome.n_opcoes = 2;
  icm_metronome.select = 0;
  strcpy(icm_metronome.opcoes[LIGAR], " LIGAR  ");
  strcpy(icm_metronome.opcoes[DESLIGAR], "DESLIGAR");
}

void RCE() {
  cancel_repeating_timer(&general_timer);
  oled_draw_string(ssd, 0, 56, "          ", false);
  print_conduta(MSG_RCE);
  print_menu(MENU_DESLIGADO);
}

// frequencia de ligado/desligado do coração
bool heart_beat_frequency_callback(struct repeating_timer *t) {
  flip_flop = !flip_flop;
  set_buzzer(flip_flop);
  set_heart(flip_flop);
  add_alarm_in_ms(100, metronome_off_callback, NULL, false);
  return true;
}

// frequencia de ligado/desligado do Alerta
bool alert_alarm_frequency_callback(struct repeating_timer *t) {
  flip_flop = !flip_flop;
  set_buzzer(flip_flop);
  set_light(flip_flop);
  add_alarm_in_ms(100, metronome_off_callback, NULL, false);
  return true;
}

// desliga o buzzer e a luz
int64_t metronome_off_callback(struct repeating_timer *t) {
  set_buzzer(OFF);
  set_light(OFF);
  return 0;
}

// Modifica a frequencia original do buzzer (buzzer_freq).
void set_buzzer_frequency(uint16_t freq) {
  uint gpio_buzzer_slice = pwm_gpio_to_slice_num(buzzer);  // descobre qual o Canal (slice) de frequencia desta porta
     float divisor = clock_get_hz(clk_sys)/(freq*(buzzer_top+1)); 
        if (divisor > 256) divisor = 256;
        if (divisor <= 1) divisor = 1; 
     pwm_set_clkdiv(gpio_buzzer_slice, divisor); 
}

void set_metronome(bool status) {
  if (status == ON) {
    if (metronome_timer.alarm_id == 0) { // isso evita que se ligue mais de uma vez enquanto ainda está ativo
      set_buzzer_frequency(800);
      add_repeating_timer_ms(545/2, heart_beat_frequency_callback, NULL, &metronome_timer);  // 545ms = freq. aproximada de 110 compressões por min
    }
  } else {
    cancel_repeating_timer(&metronome_timer);
    set_buzzer(false);
    clear_ws2812();
  }
}

void ws2812_send(uint32_t color) {
  pio_sm_put_blocking(pio, sm, color << 8);
}

void clear_ws2812() {
  uint32_t black = 0x000000; // Apaga todos os LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
      ws2812_send(black);
  }
}

void set_ws2812(uint32_t colors[NUM_LEDS]) {
  for (int i = 0; i < NUM_LEDS; i++) {
      ws2812_send(colors[i]);
  }
}

void set_heart(bool status) {
  uint32_t RED = 0x000600, BLUE = 0x000006, GREEN = 0x060000, WHITE = 0x060606, BLACK = 0x000000;
  if (status == true) {
    uint32_t matrix_colors[NUM_LEDS] = {
      BLACK, BLACK, RED, BLACK, BLACK,  // Quinta linha
      BLACK, RED, BLACK, RED, BLACK, // Quarta linha
      RED, BLACK, BLACK, BLACK, RED, // Terceira linha
      RED, BLACK, RED, BLACK, RED, // Segunda linha
      BLACK, RED, BLACK, RED, BLACK, // Primeira linha
    };
    set_ws2812(matrix_colors);
  } else {
    clear_ws2812();
    // matrix_colors[NUM_LEDS] = {
    //   BLACK, BLACK, RED, BLACK, BLACK,  // Quinta linha
    //   BLACK, RED, BLACK, RED, BLACK, // Quarta linha
    //   RED, BLACK, BLACK, BLACK, RED, // Terceira linha
    //   RED, BLACK, RED, BLACK, RED, // Segunda linha
    //   BLACK, RED, BLACK, RED, BLACK, // Primeira linha
    // };
  }
}

int64_t cancel_alarm_callback() {
  set_light(OFF);
  set_buzzer(OFF);
  return 0;
}

bool alert_callback(repeating_timer_t *t) {
  uint8_t *counter = (uint8_t *)t->user_data;  
  counter[0]++;  // Incrementa a contagem de segundos
  flip_flop = !flip_flop;
  set_light(flip_flop);
  set_buzzer(flip_flop);
  //printf("Alerta ativo! Tempo: %d/%d segundos\n", counter[0], counter[1]);

  // Se atingir as repetições todas, para o timer
  if (counter[0] >= counter[1]) {
    if (counter[2] == true) {
      set_light(ON);
      set_buzzer(ON);
      add_alarm_in_ms(1000, cancel_alarm_callback, NULL, false);
    }
    cancel_repeating_timer(&alert_timer);
    return false;  // Para o timer
  }
  return true;  // Continua o timer
}

void alert(uint8_t n_bips, uint16_t frequencia_ms, bool final_beep) { 
  alert_counter[0] = 0; // contador
  alert_counter[1] = n_bips*2; // numero de chamadas ao callback total
  alert_counter[2] = final_beep; // se tem ou não bip longo no final
  add_repeating_timer_ms(frequencia_ms/2, alert_callback, alert_counter, &alert_timer);
}