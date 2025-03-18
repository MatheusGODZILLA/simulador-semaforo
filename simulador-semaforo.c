#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2818b.pio.h"

#define LED_GREEN 11   // GPIO conectado ao terminal verde do LED RGB
#define LED_RED 13  // GPIO conectado ao terminal verde do LED RGB
#define BUTTON_A 5    // GPIO conectado ao Botão A
#define BUTTON_B 6    // GPIO conectado ao Botão B

#define LED_PIN 7      // GPIO da matriz de LEDs
#define LED_COUNT 25   // Número total de LEDs na matriz

typedef struct {
    uint8_t G, R, B;  // Formato GRB
} npLED_t;

npLED_t leds[LED_COUNT]; // Buffer dos LEDs
PIO np_pio;
uint np_sm;

uint32_t last_time_vermelho = 0;
uint32_t last_time_verde = 0;
uint32_t last_time_amarelo = 0;
uint32_t last_button_check = 0;

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

// Define a cor de todos os LEDs
void setColor(uint8_t r, uint8_t g, uint8_t b) {
    for (uint i = 0; i < LED_COUNT; i++) {
        leds[i].R = r;
        leds[i].G = g;
        leds[i].B = b;
    }
}

//Atualiza os LEDs da matriz
void npWrite() {
    for (uint i = 0; i < LED_COUNT; i++) {
        pio_sm_put_blocking(np_pio, np_sm, leds[i].G);
        pio_sm_put_blocking(np_pio, np_sm, leds[i].R);
        pio_sm_put_blocking(np_pio, np_sm, leds[i].B);
    }
    sleep_us(100); // Tempo de reset
}

void semaforo_vermelho() {
    if (time_us_32() - last_time_vermelho > 50000000) {
        setColor(255, 0, 0);
        npWrite();
        last_time_vermelho = time_us_32();
    }
}

void semaforo_verde() {
    if (time_us_32() - last_time_verde > 60000000) {
        setColor(0, 255, 0);
        npWrite();
        last_time_verde = time_us_32();
    }
}

void semaforo_amarelo() {
    if (time_us_32() - last_time_amarelo > 25000000) {
        setColor(255, 255, 0);
        npWrite();
        last_time_amarelo = time_us_32();
    }
}

void desligar_leds() {
    for (uint i = 0; i < LED_COUNT; i++) {
        setColor(0, 0, 0);
    }
}

void setup_matriz(uint pin, uint amount) {
    printf("[LOG] Inicializando matriz de LEDs...\n");
    np_pio = pio0;
    uint offset = pio_add_program(np_pio, &ws2818b_program);
    np_sm = pio_claim_unused_sm(np_pio, true);
    
    ws2818b_program_init(np_pio, np_sm, offset, LED_PIN, 800000.f);

    // Limpa todos os LEDs
    for (uint i = 0; i < LED_COUNT; i++) {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }

    desligar_leds(); 
}

int main() {
    stdio_init_all();  // Inicializa a comunicação serial
    desligar_leds();
    sleep_ms(2000);  // Aguarda 2 segundos para a inicialização do terminal

    printf("[LOG] Inicializando semáforo...\n");

    setup_leds();
    setup_buttons();
    setup_matriz(LED_PIN, LED_COUNT);

    while (true) {
        if (!gpio_get(BUTTON_A)) {
            printf("Botão A pressionado (Acelerar)\n");
            gpio_put(LED_GREEN, 1);
            gpio_put(LED_RED, 0);
            last_button_check = time_us_32();
        } else {
            gpio_put(LED_GREEN, 0);
            gpio_put(LED_RED, 0);
        }
    
        if (!gpio_get(BUTTON_B)) {
            printf("Botão B pressionado (Frear)\n");
            gpio_put(LED_GREEN, 0);
            gpio_put(LED_RED, 1);
            last_button_check = time_us_32();
        } else {
            gpio_put(LED_GREEN, 0);
            gpio_put(LED_RED, 0);
        }
        
        // Controle do semáforo
        semaforo_vermelho();
        semaforo_amarelo();  
        semaforo_verde();  

        // Pequeno delay para evitar leituras inconsistentes (debounce simples)
        sleep_ms(20);
    }
}
