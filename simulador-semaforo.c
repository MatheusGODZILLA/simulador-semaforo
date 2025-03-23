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

// Estados do semáforo
typedef enum {
    SEMAFORO_VERDE,
    SEMAFORO_AMARELO,
    SEMAFORO_VERMELHO
} EstadoSemaforo;

EstadoSemaforo estado_atual = SEMAFORO_VERMELHO;

// Estado e controle
int pontuacao = 0;
bool botao_a_liberado = true;
bool botao_b_liberado = true;
bool acionou_botao_durante_amarelo = false;
bool manteve_acelerando = false;
bool manteve_freando = false;
bool acao_valida = false;

char ultima_acao[22] = "";
char feedback[22] = "";

uint32_t tempo_estado_anterior = 0;

//--------------------   Setup dos componentes  --------------------//
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

// Define a cor de todos os LEDs da matriz
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

    setColor(0, 0, 0);
    npWrite();
}

void display_text(char *lines[], uint line_count) {
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);  // Limpa o display

    int y = (ssd1306_n_pages * 8 - (line_count * 8)) / 2; // Ajuste centralizado verticalmente
    for (uint i = 0; i < line_count; i++) {
        ssd1306_draw_string(ssd, 10, y, lines[i]);  // Ajustado para não cortar
        y += 10; // Espaçamento maior para evitar sobreposição
    }
    render_on_display(ssd, &frame_area); 
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
        "  Simulador  ",
        " de Semaforo "
    };

    display_text(text, 2);
}

void atualizar_display_info() {
    char linha1[22];
    char linha2[22];
    sprintf(linha1, "Pont %d", pontuacao);
    sprintf(linha2, "Sinal %s",
        estado_atual == SEMAFORO_VERDE ? "VERDE" :
        estado_atual == SEMAFORO_AMARELO ? "AMARELO" : "VERMELHO"
    );

    char *msg[] = {
        linha1,
        linha2,
        ultima_acao[0] ? ultima_acao : "Aguardando",
        feedback[0] ? feedback : ""
    };
    display_text(msg, 4);
}

//--------------------   Lógica do jogo  --------------------//
void start_simulator() {
    char *msg[] = {
        "  Pressione   ",
        "    A e B     ",
        " Para Iniciar "
    };
    display_text(msg, 3);

    while (true) {
        // Verifica se ambos os botões foram pressionados simultaneamente
        if (!gpio_get(BUTTON_A) && !gpio_get(BUTTON_B)) {
            sleep_ms(500);
            while (!gpio_get(BUTTON_A) && !gpio_get(BUTTON_B)) {
                sleep_ms(50);
            }
            break;
        }
        sleep_ms(20);
    }
}

void atualizar_semaforo() {
    uint32_t agora = time_us_32();

    switch (estado_atual) {
        case SEMAFORO_VERDE:
            if (agora - tempo_estado_anterior > TEMPO_VERDE) {
                if (manteve_acelerando) {
                    pontuacao++;
                    strcpy(ultima_acao, "Acelerou VERDE");
                    strcpy(feedback, "Correto");
                } else {
                    strcpy(ultima_acao, "Nao acelerou");
                    strcpy(feedback, "Sem ponto");
                }
                manteve_acelerando = false;

                estado_atual = SEMAFORO_AMARELO;
                setColor(141, 141, 0);
                npWrite();
                tempo_estado_anterior = agora;
            }
            break;

        case SEMAFORO_AMARELO:
            if (agora - tempo_estado_anterior > TEMPO_AMARELO) {
                if (!manteve_acelerando && !manteve_freando) {
                    pontuacao++;
                    strcpy(ultima_acao, "Esperou AMARELO");
                    strcpy(feedback, "Correto");
                } else {
                    strcpy(ultima_acao, "Agiu AMARELO");
                    strcpy(feedback, "Sem ponto");
                }

                estado_atual = SEMAFORO_VERMELHO;
                setColor(141, 0, 0);
                npWrite();
                tempo_estado_anterior = agora;
            }
            break;

        case SEMAFORO_VERMELHO:
            if (agora - tempo_estado_anterior > TEMPO_VERMELHO) {
                if (manteve_freando) {
                    pontuacao++;
                    strcpy(ultima_acao, "Freou VERMELHO");
                    strcpy(feedback, "Correto");
                } else {
                    strcpy(ultima_acao, "Nao freou");
                    strcpy(feedback, "Sem ponto");
                }
                manteve_freando = false;

                estado_atual = SEMAFORO_VERDE;
                setColor(0, 141, 0);
                npWrite();
                tempo_estado_anterior = agora;
            }
            break;
    }
}

void avaliar_acao(const char *acao) {
    acao_valida = false;

    if (strcmp(acao, "acelerar") == 0) {
        strcpy(ultima_acao, "Vc acelerou");
        
        if (estado_atual == SEMAFORO_VERDE) {
            strcpy(feedback, "Mantenha");
            acao_valida = true;
        } else {
            pontuacao--;
            strcpy(feedback, "ERRO");
        }
    } else if (strcmp(acao, "frear") == 0) {
        strcpy(ultima_acao, "Vc freou");
        
        if (estado_atual == SEMAFORO_VERMELHO) {
            strcpy(feedback, "Mantenha");
            acao_valida = true;
        } else {
            pontuacao--;
            strcpy(feedback, "ERRO");
        }
    }
}

int main() {
    stdio_init_all();
    setup_oled();
    desligar_leds();
    sleep_ms(5000);

    setup_leds();
    setup_buttons();
    setup_matriz(LED_PIN, LED_COUNT);

    while (true) { 
        start_simulator();

        printf("[LOG] Iniciando o semáforo...\n");

        estado_atual = SEMAFORO_VERDE;
        setColor(0, 141, 0);
        npWrite();
        tempo_estado_anterior = time_us_32();
        pontuacao = 0;
        acionou_botao_durante_amarelo = false;

        while (true) {
            if (!gpio_get(BUTTON_A) && !gpio_get(BUTTON_B)) {
                printf("[LOG] Encerrando o simulador...\n");
                sleep_ms(500);
                while (!gpio_get(BUTTON_A) && !gpio_get(BUTTON_B)) {
                    sleep_ms(50);
                }
                desligar_leds();
                break; 
            }

            // Controles dos botões
            if (!gpio_get(BUTTON_A)) {
                if (botao_a_liberado) {
                    avaliar_acao("acelerar");
                    botao_a_liberado = false;
                    gpio_put(LED_GREEN, 1);
                    gpio_put(LED_RED, 0);
                    manteve_acelerando = true;
                }
                if (estado_atual == SEMAFORO_AMARELO)
                    acionou_botao_durante_amarelo = true;
            } else {
                botao_a_liberado = true;
                gpio_put(LED_GREEN, 0);
                gpio_put(LED_RED, 0);
            }

            // Botao B
            if (!gpio_get(BUTTON_B)) {
                if (botao_b_liberado) {
                    avaliar_acao("frear");
                    botao_b_liberado = false;
                    gpio_put(LED_GREEN, 0);
                    gpio_put(LED_RED, 1);
                    manteve_freando = true;
                }
                if (estado_atual == SEMAFORO_AMARELO)
                    acionou_botao_durante_amarelo = true;
            } else {
                botao_b_liberado = true;
                gpio_put(LED_GREEN, 0);
                gpio_put(LED_RED, 0);
            }

            // Atualiza OLED com pontuação, estado do semáforo e última ação
            atualizar_display_info();

            // Controle do semáforo
            atualizar_semaforo();  

            sleep_ms(20);
        }

        char resultado[22];
        sprintf(resultado, " Pontuacao %d", pontuacao);
        char *msg[] = { " Simulador  ", 
                        " Encerrado ", resultado };
        display_text(msg, 3);
        sleep_ms(3000);
    }
}
