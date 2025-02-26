#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define WS2812_PIN 7
#define BTN_A 5
#define BTN_B 6

#define IS_RGBW false
#define NUM_PIXELS 25

volatile int numero_exibido = 0;
volatile bool atualizar_display = true;
volatile uint32_t ultimo_tempo_interrupcao = 0;

ssd1306_t ssd;
volatile int primeiro_numero = -1;
volatile int segundo_numero = -1;
volatile int resultado = 0;

const uint32_t padroes_numeros[10][25] = { 
    /* Padrões dos números omitidos por brevidade */
 };

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void atualizar_matriz() {
    uint32_t cor = urgb_u32(160, 32, 240);
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(padroes_numeros[numero_exibido][i] ? cor : 0);
    }
}

void atualizar_display_oled(int resultado) {
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, "RESULTADO", 8, 10);
    char buffer[10];
    sprintf(buffer, "%d", resultado);
    ssd1306_draw_string(&ssd, buffer, 20, 30);
    ssd1306_send_data(&ssd);
}

void interrupcao_botao(uint gpio, uint32_t eventos) {
    if (gpio == BTN_A) {
        numero_exibido = (numero_exibido + 1) % 10;
        atualizar_display = true; // Forçar atualização da matriz de LEDs
    } else if (gpio == BTN_B) {
        if (primeiro_numero == -1) {
            primeiro_numero = numero_exibido;
            numero_exibido = 0;
            atualizar_display = true; // Forçar atualização da matriz de LEDs
        } else if (segundo_numero == -1) {
            segundo_numero = numero_exibido;
            resultado = primeiro_numero * segundo_numero;
            atualizar_display_oled(resultado);
            primeiro_numero = -1;
            segundo_numero = -1;
            numero_exibido = 0;
            atualizar_display = true; // Forçar atualização da matriz de LEDs
        }
    }
}

void inicializar_componentes() {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    gpio_init(BTN_A);
    gpio_init(BTN_B);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_A);
    gpio_pull_up(BTN_B);

    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &interrupcao_botao);
    gpio_set_irq_enabled(BTN_B, GPIO_IRQ_EDGE_FALL, true);
}

int main() {
    stdio_init_all();
    printf("Iniciando a calculadora de multiplicação\n");
    atualizar_display_oled(0);

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    inicializar_componentes();

    while (1) {
        if (atualizar_display) {
            atualizar_matriz();
            atualizar_display = false;
        }
        sleep_ms(100);
    }
}