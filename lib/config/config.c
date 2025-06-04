#include "config.h"

// PIO pio = pio0;
// int sm = 0;

void btn_setup() {
    gpio_init(BTNA);
    gpio_set_dir(BTNA, GPIO_IN);
    gpio_pull_up(BTNA);
}

void output_setup() {
    gpio_init(GREEN);
    gpio_set_dir(GREEN, GPIO_OUT);
    gpio_init(RED);
    gpio_set_dir(RED, GPIO_OUT);
    gpio_init(BLUE);
    gpio_set_dir(BLUE, GPIO_OUT);
    gpio_init(HEATER);
    gpio_set_dir(HEATER, GPIO_OUT);
    gpio_init(REFRIGER);
    gpio_set_dir(REFRIGER, GPIO_OUT);
    gpio_init(FAN_1);
    gpio_set_dir(FAN_1, GPIO_OUT);
    gpio_init(FAN_2);
    gpio_set_dir(FAN_2, GPIO_OUT);
    gpio_init(LIGHT);
    gpio_set_dir(LIGHT, GPIO_OUT);
    gpio_init(CURTAIN);
    gpio_set_dir(CURTAIN, GPIO_OUT);
}

// Inicializa e configura a comunicação serial I2C 
void i2c_setup() {
    i2c_init(I2C_PORT, 4e2 * 1e3);              // Inicialização I2C.
  
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);  // Define a função do pino GPIO para I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);  // Define a função do pino GPIO para I2C
    gpio_pull_up(I2C_SDA);                      // Pull up para linha de dados
    gpio_pull_up(I2C_SCL);                      // Pull up para linha de clock
}

// Limpa o display
void clear(ssd1306_t* ssd) {
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}

// Inicializa e configura o display
void ssd1306_setup(ssd1306_t* ssd) {
    ssd1306_init(ssd, WIDTH, HEIGHT, false, I2C_address, I2C_PORT); 
    ssd1306_config(ssd); 
    ssd1306_send_data(ssd);
    clear(ssd);
    ssd1306_send_data(ssd);
}

// // Inicializa e configura a matriz de LEDs 5x5 
// void ws2812_setup(PIO pio, uint sm) {
//     uint offset = pio_add_program(pio, &ws2812_program);
  
//     ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW); //Inicializa a matriz de leds
// }

// Configura o PWM no pino do buzzer com uma frequência especificada
void set_buzzer_frequency(uint frequency) {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER);
    gpio_set_function(BUZZER, GPIO_FUNC_PWM);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (frequency * 4096));
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(BUZZER, 0);
}

// Função para tocar o buzzer por um tempo especificado (em milissegundos)
void play_buzzer(uint frequency) {
    set_buzzer_frequency(frequency);
    pwm_set_gpio_level(BUZZER, 32768);
}

void stop_buzzer() {
    pwm_set_gpio_level(BUZZER, 0);
}

// Inicializa e configura os pinos do joystick como periféricos ADC
void adc_setup() {
    adc_init();
    adc_gpio_init(LDR);
}

void init(/*PIO pio, uint sm,*/  ssd1306_t* ssd) {
    stdio_init_all();
    btn_setup();
    output_setup();
    i2c_setup();
    ssd1306_setup(ssd);
    adc_setup();
}