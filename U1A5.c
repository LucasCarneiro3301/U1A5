#include "pico/stdlib.h"            // Biblioteca da Raspberry Pi Pico para funções padrão (GPIO, temporização, etc.)
#include "pico/unique_id.h"         // Biblioteca com recursos para trabalhar com os pinos GPIO do Raspberry Pi Pico

#include "hardware/gpio.h"          // Biblioteca de hardware de GPIO
#include "hardware/irq.h"           // Biblioteca de hardware de interrupções
#include "hardware/adc.h"           // Biblioteca de hardware para conversão ADC

#include "./lib/cyw43/cyw43.h"
#include "./lib/mqtt/mqtt.h"
#include "./lib/config/config.h"
#include "./lib/tasks/tasks.h"

SemaphoreHandle_t xDisplayMut;     // Mutex para proteger o acesso ao display
SemaphoreHandle_t xStopSem;         // Semáforo binário usado para sinalizar a tarefa de reset

ssd1306_t ssd;
dht_result_t result;
float temperature;
float humidity;
float lux;
float q_temp, q_humid, q_lux;
float quality;
bool stop = false;

volatile uint32_t last_time = 0;    // Armazena o último tempo em microssegundos

void gpio_irq_handler(uint gpio, uint32_t events);

int main(void) {
    char str[16];

    init(&ssd);

    ssd1306_fill(&ssd, false);  					    // Limpa o display
    ssd1306_draw_string(&ssd, "AGUARDE", 28, 28);	    // Desenha uma string 
    ssd1306_draw_string(&ssd, "A CONEXAO...", 24, 40);  // Desenha uma string
    ssd1306_send_data(&ssd);    					    // Atualiza o display

    // Inicializa e configura o CI CYW43
    if(cyw43_setup() != 1) {
        ssd1306_fill(&ssd, false);  					    // Limpa o display
        ssd1306_draw_string(&ssd, "CONEXAO", 28, 28);	    // Desenha uma string 
        ssd1306_draw_string(&ssd, "MAL SUCEDIDA", 24, 40);	// Desenha uma string
        ssd1306_send_data(&ssd);    					    // Atualiza o display

        return -1;
    }


    gpio_set_irq_enabled_with_callback(BTNA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);   // Ativa a interrupção do botão de parada na borda de descida

    xDisplayMut = xSemaphoreCreateMutex();
    xStopSem = xSemaphoreCreateBinary();

    xTaskCreate(vTaskDisplay, "Display Task", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);           // Exibe temp, umidade, luminosidade e qualidade
    xTaskCreate(vTaskDHT, "DHT Task", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);                   // Mede temp e umidade
    xTaskCreate(vTaskLDR, "LDR Task", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);                   // Mede luminosidade
    xTaskCreate(vTaskMQTTClient, "MQTT Client Task", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);    // Cliente MQTT
    xTaskCreate(vTaskActuator, "Actuators Task", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);        // Aciona os atuadores
    xTaskCreate(vTaskGMF, "GMF Task", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);                   // Função de pertinência gaussiana nos valores
    xTaskCreate(vTaskStatus, "STATUS Task", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);             // Status

    vTaskStartScheduler();
    panic_unsupported();
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());  // Obtém o tempo atual em microssegundos

    if (current_time - last_time > 5e5) {   // 200 ms de debouncing
        last_time = current_time; 
        if (gpio == BTNA) {
            stop = !stop;
        }
    }
}