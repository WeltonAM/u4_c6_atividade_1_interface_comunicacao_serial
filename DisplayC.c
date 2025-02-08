#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
#define ENDERECO 0x3C
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

// Estruturas para LEDs
typedef struct
{
    uint pin;
    bool state;
    const char *color_name;
    uint8_t r, g, b;
} Led;

Led leds[] = {
    {LED_PIN_GREEN, false, "Verde", 0, 255, 0},
    {LED_PIN_BLUE, false, "Azul", 0, 0, 255},
    {LED_PIN_RED, false, "Vermelho", 255, 0, 0}};

// Inicialização do Display SSD1306
void init_display(ssd1306_t *ssd)
{
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(ssd);
    ssd1306_send_data(ssd);
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}

// Inicializa todos os LEDs RGB
void init_rgb_leds()
{
    for (int i = 0; i < 3; i++)
    {
        gpio_init(leds[i].pin);
        gpio_set_dir(leds[i].pin, GPIO_OUT);
    }
}

// Inicializa os botões
void init_buttons()
{
    gpio_init(BTN_A_PIN);
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    gpio_pull_up(BTN_B_PIN);
}

// Função de Debouncing para botões
bool debounce(uint pin)
{
    static uint64_t last_interrupt_time_A = 0;
    static uint64_t last_interrupt_time_B = 0;

    uint64_t interrupt_time = time_us_32();
    uint64_t *last_time = (pin == BTN_A_PIN) ? &last_interrupt_time_A : &last_interrupt_time_B;

    if (interrupt_time - *last_time > 200000)
    {
        *last_time = interrupt_time;
        return true;
    }
    return false;
}

// Função para atualizar o estado de um LED e a tela
void update_led_and_display(ssd1306_t *ssd, int led_index)
{
    Led *led = &leds[led_index];

    // Alterna o estado do LED
    led->state = !led->state;
    gpio_put(led->pin, led->state);

    // Exibe a mensagem no terminal
    printf("Botão %s pressionado. LED %s %s\n", led->color_name, led->color_name, led->state ? "Ligado" : "Desligado");

    // Atualiza a tela OLED
    char message[32];
    sprintf(message, "LED %s", led->color_name); // Exibe o nome do LED
    ssd1306_fill(ssd, false);
    ssd1306_draw_string(ssd, message, 10, 10); // Primeira linha com o nome do LED

    // Exibe o estado do LED (Ligado ou Desligado)
    sprintf(message, "%s", led->state ? "Ligado" : "Desligado");
    ssd1306_draw_string(ssd, message, 10, 20); // Segunda linha com o estado do LED

    ssd1306_send_data(ssd); // Atualiza o display
}

// Função de interrupção para botões
void button_isr(uint gpio, uint32_t events)
{
    static ssd1306_t ssd;
    static bool display_initialized = false;

    if (!display_initialized)
    {
        init_display(&ssd); // Inicializa o display SSD1306
        display_initialized = true;
    }

    // Verifica qual botão foi pressionado e atualiza o LED correspondente
    if (gpio == BTN_A_PIN && debounce(BTN_A_PIN))
    {
        update_led_and_display(&ssd, 0); // LED verde
    }
    else if (gpio == BTN_B_PIN && debounce(BTN_B_PIN))
    {
        update_led_and_display(&ssd, 1); // LED azul
    }
}

// Configuração das interrupções de botões
void setup_interrupts()
{
    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, button_isr);
    gpio_set_irq_enabled_with_callback(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true, button_isr);
}

// Função para enviar caracteres via UART
void send_char_via_uart(char c)
{
    uart_putc(uart0, c);
}

// Função para enviar um pixel para o WS2812
static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

// Função para converter cores para o formato GRB
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

// Função para exibir o número na matriz de LEDs WS2812
void display_number(int num)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        led_buffer[i] = urgb_u32(0, 0, 0); // Todos os LEDs apagados
    }

    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (num_map[num][i] == 1)
        {
            led_buffer[i] = random_color(); // Cores aleatórias para testes na placa BitDogLab
            // led_buffer[i] = urgb_u32(255, 0, 0); // Leds mais fortes para teste no simulador Wokwi
        }
    }

    for (int i = 0; i < NUM_PIXELS; i++)
    {
        put_pixel(led_buffer[i]);
    }
}

int main()
{
    stdio_init_all();

    init_buttons();
    init_rgb_leds();

    ssd1306_t ssd;
    init_display(&ssd);

    setup_interrupts();

    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    printf("\n--- Bem vindo! ---\n Digite um valor para exibir no display\n");

    while (true)
    {
        if (stdio_usb_connected())
        {
            char c;
            if (scanf("%c", &c) == 1)
            {
                printf("Valor recebido: '%c'\n", c);
                ssd1306_draw_char(&ssd, c, 20, 20);
                ssd1306_send_data(&ssd);

                if (c >= '0' && c <= '9')
                {
                    display_number(c - '0');
                }
            }
        }

        sleep_ms(100); // Atraso para reduzir uso de CPU
    }

    return 0;
}
