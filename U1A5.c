#include "pico/stdlib.h"            // Biblioteca da Raspberry Pi Pico para funções padrão (GPIO, temporização, etc.)
#include "pico/unique_id.h"         // Biblioteca com recursos para trabalhar com os pinos GPIO do Raspberry Pi Pico

#include "hardware/gpio.h"          // Biblioteca de hardware de GPIO
#include "hardware/irq.h"           // Biblioteca de hardware de interrupções
#include "hardware/adc.h"           // Biblioteca de hardware para conversão ADC
#include <dht.h>

#include "./lib/cyw43/cyw43.h"
#include "./lib/mqtt/mqtt.h"
#include "./lib/config/config.h"

#define DHT_PIN 16

static const dht_model_t dht_model = DHT11;
dht_t dht;

int main(void) {
    char str[16];
    ssd1306_t ssd;
    static MQTT_CLIENT_DATA_T state;    // Cria registro com os dados do cliente

    init(&ssd);
    dht_init(&dht, dht_model, pio0, DHT_PIN, true);

    // Inicializa e configura o CI CYW43
    if(cyw43_setup() != 1) {
        ssd1306_fill(&ssd, false);  					// Limpa o display
        ssd1306_draw_string(&ssd, "CONEXAO", 28, 28);	// Desenha uma string 
        ssd1306_draw_string(&ssd, "MAL SUCEDIDA", 24, 40);	// Desenha uma string
        ssd1306_send_data(&ssd);    					// Atualiza o display

        return -1;
    }

    mqtt_setup(&state);

    //Faz um pedido de DNS para o endereço IP do servidor MQTT
    cyw43_arch_lwip_begin();
    int err = dns_gethostbyname(MQTT_SERVER, &state.mqtt_server_address, dns_found, &state);
    cyw43_arch_lwip_end();

    // Se tiver o endereço, inicia o cliente
    if (err == ERR_OK) {
        start_client(&state);
    } else if (err != ERR_INPROGRESS) { // ERR_INPROGRESS means expect a callback
        panic("dns request failed");
    }

    // Loop condicionado a conexão mqtt
    while (!state.connect_done || mqtt_client_is_connected(state.mqtt_client_inst)) {
        ssd1306_fill(&ssd, false);                           // Limpa o display
        ssd1306_rect(&ssd, 1, 1, 123, 63, true, false);       // Retângulo da área útil
        sprintf(str, "TEMPC:%.2f(C)", temperature);
        ssd1306_draw_string(&ssd, str ,4, 8);
        sprintf(str, "HUMID:%.2f(%%)", humidity);
        ssd1306_draw_string(&ssd, str,4, 24);
        ssd1306_draw_string(&ssd, "QUALITY: -",4, 47);
        ssd1306_line(&ssd, 1, 36, 123, 36, true);		    // Desenha uma linha
        ssd1306_send_data(&ssd);                            // Atualiza o display
        
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(10000));
    }

    printf("mqtt client exiting\n");
    return 0;
}