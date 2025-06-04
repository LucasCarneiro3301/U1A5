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
SemaphoreHandle_t xResetSem;       // Semáforo binário usado para sinalizar a tarefa de reset

ssd1306_t ssd;
dht_result_t result;
float temperature;
float humidity;
float lux;

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

    xDisplayMut = xSemaphoreCreateMutex();
    xResetSem = xSemaphoreCreateBinary();

    xTaskCreate(vTaskDisplay, "Display Task", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);           // Exibe temp, umidade, luminosidade e qualidade
    xTaskCreate(vTaskDHT, "DHT Task", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);                   // Mede temp e umidade
    xTaskCreate(vTaskLDR, "LDR Task", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);                   // Mede luminosidade
    xTaskCreate(vTaskMQTTClient, "MQTT Client Task", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);    // Cliente MQTT

    vTaskStartScheduler();
    panic_unsupported();
}