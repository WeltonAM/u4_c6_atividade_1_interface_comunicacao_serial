#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "inc/numbers.h"
#include "ws2812.pio.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define NUM_PIXELS 25

#define LED_PIN_GREEN 11
#define LED_PIN_BLUE 12
#define LED_PIN_RED 13
#define BTN_A_PIN 5
#define BTN_B_PIN 6
#define WS2812_PIN 7

PIO pio = pio0;
int sm = 0;
uint32_t led_buffer[NUM_PIXELS] = {0};

bool led_green_state = false;
bool led_blue_state = false;
bool led_red_state = false;

// Função para inicializar o I2C e o display
void init_display(ssd1306_t *ssd) {
    i2c_init(I2C_PORT, 400 * 1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    ssd1306_init(ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(ssd);
    
    ssd1306_send_data(ssd);
    ssd1306_fill(ssd, false);
    
    ssd1306_send_data(ssd);
}

// Função para inicializar LEDs comuns (RGB)
void init_rgb_leds() {
    gpio_init(LED_PIN_GREEN);
    gpio_init(LED_PIN_BLUE);
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
}

// Função para configurar os botões
void init_buttons() {
    gpio_init(BTN_A_PIN);
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    gpio_pull_up(BTN_B_PIN);
}

// Função de Debouncing
bool debounce(uint pin) {
    static uint64_t last_interrupt_time_A = 0;
    static uint64_t last_interrupt_time_B = 0;
    
    uint64_t interrupt_time = time_us_32();
    uint64_t *last_time = (pin == BTN_A_PIN) ? &last_interrupt_time_A : &last_interrupt_time_B;

    if (interrupt_time - *last_time > 200000) { 
        *last_time = interrupt_time;
        return true;
    }
    
    return false;
}

void button_isr(uint gpio, uint32_t events) {
    if (gpio == BTN_A_PIN && debounce(BTN_A_PIN)) {
        led_green_state = !led_green_state;
        gpio_put(LED_PIN_GREEN, led_green_state);
        printf("Botão A pressionado. LED Verde %s\n", led_green_state ? "Ligado" : "Desligado");
    } else if (gpio == BTN_B_PIN && debounce(BTN_B_PIN)) {
        led_blue_state = !led_blue_state;
        gpio_put(LED_PIN_BLUE, led_blue_state);
        printf("Botão B pressionado. LED Azul %s\n", led_blue_state ? "Ligado" : "Desligado");
    }
}

void setup_interrupts() {
    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, button_isr);
    gpio_set_irq_enabled_with_callback(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true, button_isr);
}

// Função de envio de caracteres pelo Serial Monitor
void send_char_via_uart(char c) {
    uart_putc(uart0, c);
}

// Função para enviar um pixel para o WS2812
static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

// Função que converte as cores para o formato GRB
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Função que gera uma cor aleatória
static inline uint32_t random_color()
{
    uint8_t r = rand() % 20;
    uint8_t g = rand() % 20;
    uint8_t b = rand() % 20;
    return urgb_u32(r, g, b);
}

// Função que define os LEDs da matriz a partir do buffer
void set_leds_from_buffer()
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        put_pixel(led_buffer[i]);
    }
}

// Função que exibe o número na matriz de LEDs e gera cores aleatórias
void display_number(int num)
{
    // Limpar todos os LEDs (para garantir que a matriz será limpa)
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        led_buffer[i] = urgb_u32(0, 0, 0); // Todos os LEDs apagados (off)
    }

    // Preencher os LEDs com base no número a ser exibido e gerar cores aleatórias para os LEDs que representam o número
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (num_map[num][i] == 1)
        {
            // Atribui uma cor aleatória ao LED que representa o número
            // led_buffer[i] = random_color();
            led_buffer[i] = urgb_u32(255, 0, 0);
        }
    }

    set_leds_from_buffer(); // Atualiza os LEDs da matriz
}

int main() {
    stdio_init_all(); // Inicializa o stdio para UART

    // Inicializa o display SSD1306
    ssd1306_t ssd;
    init_display(&ssd);

    // Inicializa botões
    init_buttons();

    // Inicializa LEDs RGB
    init_rgb_leds();

    // Configura interrupções
    setup_interrupts();

    // Inicializa o WS2812
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    char received_char;
    
    while (true) {
        // Verifica se há um caractere no monitor serial
        if (uart_is_readable(uart0)) {
            received_char = uart_getc(uart0);
            send_char_via_uart(received_char); // Envia o caractere de volta para o serial monitor

            // Exibe o caractere no display
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, &received_char, 10, 10);
            ssd1306_send_data(&ssd);

            // Se o caractere for um número, exibe na matriz WS2812
            if (received_char >= '0' && received_char <= '9') {
                display_number(received_char - '0');
                printf("Número digitado: %c\n", received_char);
            }
        }

        sleep_ms(100); // Atraso para reduzir uso de CPU
    }

    return 0;
}
