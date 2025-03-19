#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2818b.pio.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

#define LED_GREEN 11   // GPIO conectado ao terminal verde do LED RGB
#define LED_RED 13  // GPIO conectado ao terminal vermelho do LED RGB
#define BUTTON_A 5    // GPIO conectado ao Botão A
#define BUTTON_B 6    // GPIO conectado ao Botão B

#define LED_PIN 7      // GPIO da matriz de LEDs
#define LED_COUNT 25   // Número total de LEDs na matriz

#define TEMPO_VERDE  10000000  // 10 segundos
#define TEMPO_AMARELO 4000000 // 4 segundos
#define TEMPO_VERMELHO 14000000 // 14 segundos

// Configuração do I2C
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

struct render_area frame_area; // Área de renderização do display

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
    if (time_us_32() - last_time_vermelho > TEMPO_VERMELHO) {
        setColor(141, 0, 0);
        npWrite();
        last_time_vermelho = time_us_32();
    }
}

void semaforo_verde() {
    if (time_us_32() - last_time_verde > TEMPO_VERDE) {
        setColor(0, 141, 0);
        npWrite();
        last_time_verde = time_us_32();
    }
}

void semaforo_amarelo() {
    if (time_us_32() - last_time_amarelo > TEMPO_AMARELO) {
        setColor(141, 141, 0);
        npWrite();
        last_time_amarelo = time_us_32();
    }
}

void desligar_leds() {
    setColor(0, 0, 0);
    npWrite(); 
}

void setup_matriz(uint pin, uint amount) {
    printf("[LOG] Inicializando matriz de LEDs...\n");
    np_pio = pio0;
    uint offset = pio_add_program(np_pio, &ws2818b_program);
    np_sm = pio_claim_unused_sm(np_pio, true);
    
    ws2818b_program_init(np_pio, np_sm, offset, LED_PIN, 800000.f);

    setColor(0, 0, 0); // Inicializa os LEDs com cor preta
    npWrite(); // Aplica a cor inicial
}

void display_text(char *lines[], uint line_count) {
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);  // Limpa o display

    int y = 0;
    for (uint i = 0; i < line_count; i++) {
        ssd1306_draw_string(ssd, 5, y, lines[i]);
        y += 8;
    }
    render_on_display(ssd, &frame_area);  // Atualiza o display
}

void setup_oled(){
    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();

    // Preparar área de renderização para o display
    memset(&frame_area, 0, sizeof(frame_area));

    frame_area.start_column = 0;
    frame_area.end_column = ssd1306_width - 1;
    frame_area.start_page = 0;
    frame_area.end_page = ssd1306_n_pages - 1;


    calculate_render_area_buffer_length(&frame_area);

    // Zera o display inteiro
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    // Exibir texto no display OLED
    char *text[] = {
        " Bem-vindos ao ",
        " Simulador de Semáforo "
    };

    display_text(text, 2);
}

int main() {
    stdio_init_all();  // Inicializa a comunicação serial
    setup_oled();
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
        semaforo_amarelo();  
        semaforo_vermelho();
        semaforo_verde();  

        // Pequeno delay para evitar leituras inconsistentes (debounce simples)
        sleep_ms(20);
    }
}