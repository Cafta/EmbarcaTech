#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include <string.h>
#include "class/hid/hid.h"

// Função para enviar uma string como teclado HID
void send_string_as_keyboard(const char* str) {
    while (*str) {
        uint8_t keycode = 0;

        if (*str >= 'a' && *str <= 'z') {
            keycode = HID_KEY_A + (*str - 'a');
        } else if (*str >= 'A' && *str <= 'Z') {
            keycode = HID_KEY_A + (*str - 'A');
            tud_hid_keyboard_report(0, KEYBOARD_MODIFIER_LEFTSHIFT, &keycode);
            sleep_ms(10);
            tud_hid_keyboard_report(0, 0, NULL);
            sleep_ms(10);
            str++;
            continue;
        } else if (*str == ' ') {
            keycode = HID_KEY_SPACE;
        } else if (*str == '!') {
            keycode = HID_KEY_1;
            tud_hid_keyboard_report(0, KEYBOARD_MODIFIER_LEFTSHIFT, &keycode);
            sleep_ms(10);
            tud_hid_keyboard_report(0, 0, NULL);
            sleep_ms(10);
            str++;
            continue;
        } else {
            str++;
            continue;
        }

        // Envia a tecla
        tud_hid_keyboard_report(0, 0, &keycode);
        sleep_ms(10);
        tud_hid_keyboard_report(0, 0, NULL); // solta a tecla
        sleep_ms(10);
        str++;
    }

    // Envia um "Enter"
    uint8_t enter = HID_KEY_ENTER;
    tud_hid_keyboard_report(0, 0, &enter);
    sleep_ms(10);
    tud_hid_keyboard_report(0, 0, NULL);
    sleep_ms(10);
}

int main() {
    board_init();
    tusb_init();

    while (1) {
        tud_task(); // Lida com a pilha USB

        if (tud_hid_ready()) {
            send_string_as_keyboard("Hello World!");
            sleep_ms(5000);
        }
    }
}

// Requisição de relatório HID (obrigatória pelo TinyUSB)
uint16_t tud_hid_get_report_cb(uint8_t, uint8_t, uint8_t*, uint16_t) {
    return 0;
}

void tud_hid_set_report_cb(uint8_t, uint8_t, uint8_t const*, uint16_t) {
    // não usado
}
