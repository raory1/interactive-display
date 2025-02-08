#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "led_matrix.pio.h"
#include "hardware/i2c.h"
#include "include/ssd1306.h"
#include "include/font.h"
#include "include/numbers_matrix.h"

// Definindo os pinos e parâmetros de configuração
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
ssd1306_t ssd; // Inicializa a estrutura do display
bool cor = true;
volatile absolute_time_t last_press_time = 0;

// Protótipo das funções
void uart_init_custom();
void pins_init();
void atualizar_led(int pin_led, const char* linha1, const char* linha2);
void atualizar_matriz_leds(PIO pio, uint sm, int i);
void atualizar_display(const char *linha1, const char *linha2);
void gpio_irq_handler(uint gpio, uint32_t events);
uint32_t matrix_rgb(double r, double g, double b);

// Função main (principal)
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

    atualizar_matriz_leds(pio, sm, 0); // Inicializa a matriz de LEDs apagada

    // Habilita interrupções para os botões
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    char buffer[100]; // Buffer para armazenar a mensagem
    int index = 0;
    i2c_init(I2C_PORT, 400 * 1000);
    char caractere;

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                     // Pull up the data line
    gpio_pull_up(I2C_SCL);                     // Pull up the clock line

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    
    while (true) {
        atualizar_display("ISABELLA C", NULL);
        caractere = getchar();
        char str[2] = {caractere, '\0'};
        printf("Recebido: %c\n", caractere);
        if (caractere >= '0' && caractere <= '9') {
            int index = caractere - '0';  // Calcula o índice (0 a 9)
            atualizar_matriz_leds(pio, sm, index + 1);  // Chama a função com o valor correspondente (1 a 10)
        }
        else {
            atualizar_matriz_leds(pio, sm, 0); 
        }
        atualizar_display(str, NULL);
        cor = !cor;
        sleep_ms(2000);
    }
}

// Função inicializa a UART
void uart_init_custom()
{
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    printf("UART INICIALIZADA COM SUCESSO!");
}

// Função inicializa os pinos para LEDs e botões
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

// Função alterna o estado do LED
void atualizar_led(int pin_led, const char* linha1, const char* linha2)
{
    gpio_put(pin_led, !(gpio_get(pin_led)));
    atualizar_display(linha1, gpio_get(pin_led) ? "Ligado" : "Desligado");
    printf("%s %s\n", linha1, gpio_get(pin_led) ? "ligado" : "desligado");
}

// Função atualiza os LEDs da matriz para exibir o padrão atual
void atualizar_matriz_leds(PIO pio, uint sm, int i)
{
    for (int j = 0; j < 5; j++) {
        for (int k = 0; k < 5; k++) {
            uint32_t cor = matrix_rgb(numeros[i][j][k], 0, 0);
            pio_sm_put_blocking(pio, sm, cor);
        }
    }
}

// Função atualiza o display com duas linhas de texto
void atualizar_display(const char *linha1, const char *linha2)
{
    ssd1306_fill(&ssd, !cor);
    ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);
    if (linha1) {
        ssd1306_draw_string(&ssd, linha1, 20, 20);
    }
    if (linha2) {
        ssd1306_draw_string(&ssd, linha2, 8, 35);
    }
    ssd1306_send_data(&ssd);
}

// Função de interrupção para os botões
void gpio_irq_handler(uint gpio, uint32_t events)
{
    {
        bool btn_last_state = false;
    
        uint32_t current_time = to_us_since_boot(get_absolute_time());
        bool btn_pressed = !gpio_get(gpio);
    
        // Condição para evitar repetições de cliques rápidas (bouncing)
        if (btn_pressed && !btn_last_state &&
            (absolute_time_diff_us(last_press_time, get_absolute_time()) > 200000)) 
        {
            last_press_time = get_absolute_time();
            btn_last_state = true;
            if (gpio == BTN_A) {
                atualizar_led(PIN_LED_GREEN, "LED VERDE", NULL);
                printf("Botão A pressionado!\n");
                printf("-------------------\n");
            }
            else if (gpio == BTN_B) {
                atualizar_led(PIN_LED_BLUE, "LED AZUL", NULL);
                printf("Botão B pressionado!\n");
                printf("-------------------\n");
            }
        }

        else if (!btn_pressed) {
            btn_last_state = false;
        }
    }
}

// Função retorna a cor RGB para a matriz de LEDs
uint32_t matrix_rgb(double r, double g, double b)
{
    return ((uint8_t)(g * 255) << 24) | ((uint8_t)(r * 255) << 16) | ((uint8_t)(b * 255) << 8);
}
