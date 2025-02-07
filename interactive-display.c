#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "led_matrix.pio.h"
#include "hardware/i2c.h"
#include "include/ssd1306.h"
#include "include/font.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define BTN_A 5
#define BTN_B 6
#define OUT_PIN 7
#define PIN_LED_GREEN 11
#define PIN_LED_BLUE 12
#define PIN_LED_RED 13

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

PIO pio;
uint sm;

volatile int current_pattern = 0;

double numeros[11][5][5] = {
    {
        // Apagar
        {0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0},
    },
};

uint32_t matrix_rgb(double r, double g, double b)
{
    return ((uint8_t)(g * 255) << 24) | ((uint8_t)(r * 255) << 16) | ((uint8_t)(b * 255) << 8);
}

// Atualiza os LEDs da matriz para exibir o padrão atual
void atualizar_matriz_leds(PIO pio, uint sm, int current_pattern)
{
    for (int j = 0; j < 5; j++)
    {
        for (int k = 0; k < 5; k++)
        {
            uint32_t cor = matrix_rgb(numeros[current_pattern][j][k], 0, 0);
            pio_sm_put_blocking(pio, sm, cor);
        }
    }
}

void gpio_irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BTN_A)
    {
        printf("Botão A pressionado\n");
    }
    else if (gpio == BTN_B)
    {
        printf("Botão B pressionado\n");
    }
}

void uart_init_custom()
{
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    printf("INICIALIZADA A UART");
}

void pins_init()
{
    // Inicialização e configuração do LED
    gpio_init(PIN_LED_RED);
    gpio_set_dir(PIN_LED_RED, GPIO_OUT);
    gpio_init(PIN_LED_BLUE);
    gpio_set_dir(PIN_LED_BLUE, GPIO_OUT);
    gpio_init(PIN_LED_GREEN);
    gpio_set_dir(PIN_LED_GREEN, GPIO_OUT);

    // Inicialização e configuração dos botões
    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);
    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);
}

int main()
{
    stdio_init_all();
    uart_init_custom();
    pins_init();

    pio = pio0;
    set_sys_clock_khz(128000, false);
    uint offset = pio_add_program(pio, &pio_matrix_program);
    sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);

    atualizar_matriz_leds(pio, sm, current_pattern);

    // Habilita interrupções para os botões
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    char buffer[100]; // Buffer para armazenar a mensagem
    int index = 0;
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    ssd1306_t ssd; // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
  
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
  
    bool cor = true;
    while (true)
    {
      cor = !cor;
      // Atualiza o conteúdo do display com animações
      ssd1306_fill(&ssd, !cor); // Limpa o display
      ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
      ssd1306_draw_string(&ssd, "abcdefghijklmnopqrstuvwxyz", 8, 10); // Desenha uma string
      ssd1306_draw_string(&ssd, "ISINHA 123", 20, 30); // Desenha uma string
      ssd1306_draw_string(&ssd, "isinha", 15, 48); // Desenha uma string      
      ssd1306_send_data(&ssd); // Atualiza o display
  
      sleep_ms(1000);
    }
}
