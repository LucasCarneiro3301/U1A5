#ifndef MQTT_H
#define MQTT_H

#include "lwip/apps/mqtt.h"         // Biblioteca LWIP MQTT -  fornece funções e recursos para conexão MQTT
#include "lwip/apps/mqtt_priv.h"    // Biblioteca que fornece funções e recursos para Geração de Conexões
#include "lwip/dns.h"               // Biblioteca que fornece funções e recursos suporte DNS:
#include "lwip/altcp_tls.h"         // Biblioteca que fornece funções e recursos para conexões seguras usando TLS:
#include <dht.h>

#define MQTT_SERVER ""  // Substitua pelo endereço do host - broket MQTT: Ex: 192.168.1.107
#define MQTT_USERNAME ""       // Substitua pelo nome da host MQTT - Username
#define MQTT_PASSWORD ""        // Substitua pelo Password da host MQTT - credencial de acesso - caso exista

#ifndef MQTT_SERVER
#error Need to define MQTT_SERVER
#endif

// This file includes your client certificate for client server authentication
#ifdef MQTT_CERT_INC
#include MQTT_CERT_INC
#endif

#ifndef MQTT_TOPIC_LEN
#define MQTT_TOPIC_LEN 100
#endif

// Temporização da coleta de temperatura - how often to measure our temperature
#define TEMP_WORKER_TIME_S 3

// Manter o programa ativo - keep alive in seconds
#define MQTT_KEEP_ALIVE_S 60

// QoS - mqtt_subscribe
// At most once (QoS 0)
// At least once (QoS 1)
// Exactly once (QoS 2)
#define MQTT_SUBSCRIBE_QOS 1
#define MQTT_PUBLISH_QOS 1
#define MQTT_PUBLISH_RETAIN 0

// Tópico usado para: last will and testament
#define MQTT_WILL_TOPIC "/online"
#define MQTT_WILL_MSG "0"
#define MQTT_WILL_QOS 1

#ifndef MQTT_DEVICE_NAME
#define MQTT_DEVICE_NAME "pico"
#endif

// Definir como 1 para adicionar o nome do cliente aos tópicos, para suportar vários dispositivos que utilizam o mesmo servidor
#ifndef MQTT_UNIQUE_TOPIC
#define MQTT_UNIQUE_TOPIC 0
#endif

// Definição da escala de temperatura
#ifndef TEMPERATURE_UNITS
#define TEMPERATURE_UNITS 'C' // Set to 'F' for Fahrenheit
#endif

//Dados do cliente MQTT
typedef struct {
    mqtt_client_t* mqtt_client_inst;
    struct mqtt_connect_client_info_t mqtt_client_info;
    char data[MQTT_OUTPUT_RINGBUF_SIZE];
    char topic[MQTT_TOPIC_LEN];
    uint32_t len;
    ip_addr_t mqtt_server_address;
    bool connect_done;
    int subscribe_count;
    bool stop_client;
} MQTT_CLIENT_DATA_T;

extern dht_t dht;
extern float temperature;
extern float humidity;

#include "mqtt.h"
#include "pico/async_context.h"

void mqtt_setup(MQTT_CLIENT_DATA_T* state);
void pub_request_cb(__unused void *arg, err_t err);                                          // Requisição para publicar
void sub_request_cb(void *arg, err_t err);                                                   // Requisição de Assinatura - subscribe        
void unsub_request_cb(void *arg, err_t err);                                                 // Requisição para encerrar a assinatura
void sub_unsub_topics(MQTT_CLIENT_DATA_T* state, bool sub);                                  // Tópicos de assinatura
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);              // Dados de entrada MQTT
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);                  // Dados de entrada publicados
void worker_fn(async_context_t *context, async_at_time_worker_t *worker);                    // Publicar temperatura
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);  // Conexão MQTT
void start_client(MQTT_CLIENT_DATA_T *state);                                                // Inicializar o cliente MQTT
void dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg);                    // Call back com o resultado do DNS

#endif