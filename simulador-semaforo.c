#include <stdio.h>
#include "pico/stdlib.h"

#define LED_GREEN 11   // GPIO conectado ao terminal verde do LED RGB
#define LED_RED 13  // GPIO conectado ao terminal verde do LED RGB
#define BUTTON_A 5    // GPIO conectado ao Botão A
#define BUTTON_B 6    // GPIO conectado ao Botão B

void setup_leds() {
    // Configuração do GPIO do LED como saída
    gpio_init(LED_GREEN);
    gpio_init(LED_RED);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_set_dir(LED_RED, GPIO_OUT);

    // Inicialmente, os LEDs estarão apagados
    gpio_put(LED_GREEN, false);  
    gpio_put(LED_RED, false);
}

void setup_buttons() {
    // Configuração do GPIO do Botão A como entrada com pull-up interno
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    // Configuração do GPIO do Botão B como entrada com pull-up interno
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
}

int main() {
    stdio_init_all();  // Inicializa a comunicação serial

    setup_leds();  // Configuração dos LEDs
    setup_buttons();  // Configuração dos botões

    while (true) {
        if (!gpio_get(BUTTON_A)) {  // Botão A pressionado (LOW)
            printf("Botão A pressionado (Acelerar)\n");
            gpio_put(LED_GREEN, 1);  // Liga LED como feedback
            gpio_put(LED_RED, 0); // Desliga LED como feedback
            sleep_ms(200);
        } else {
            gpio_put(LED_GREEN, 0);  // Desliga LED como feedback
            gpio_put(LED_RED, 0); // Desliga LED como feedback
        }

        if (!gpio_get(BUTTON_B)) {  // Botão B pressionado (LOW)
            printf("Botão B pressionado (Frear)\n");
            gpio_put(LED_GREEN, 0);  // Desliga LED como feedback
            gpio_put(LED_RED, 1); // Liga LED como feedback
            sleep_ms(200);
        } else {
            gpio_put(LED_GREEN, 0);  // Desliga LED como feedback
            gpio_put(LED_RED, 0); // Desliga LED como feedback
        }

        // Pequeno delay para evitar leituras inconsistentes (debounce simples)
        sleep_ms(50);
    }
}
