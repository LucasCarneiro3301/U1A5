#include "tasks.h"
#include "pico/unique_id.h"         // Biblioteca com recursos para trabalhar com os pinos GPIO do Raspberry Pi Pico
#include "../cyw43/cyw43.h"
#include "../mqtt/mqtt.h"

static void pub_request_cb(__unused void *arg, err_t err);                                          // Requisição para publicar
static void sub_request_cb(void *arg, err_t err);                                                   // Requisição de Assinatura - subscribe        
static void unsub_request_cb(void *arg, err_t err);                                                 // Requisição para encerrar a assinatura
static void sub_unsub_topics(MQTT_CLIENT_DATA_T* state, bool sub);                                  // Tópicos de assinatura
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);              // Dados de entrada MQTT
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);                  // Dados de entrada publicados
static void worker_fn(async_context_t *context, async_at_time_worker_t *worker);                    // Publicar temperatura
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);  // Conexão MQTT
static void start_client(MQTT_CLIENT_DATA_T *state);                                                // Inicializar o cliente MQTT
static void dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg);                    // Call back com o resultado do DNS
static const char *full_topic(MQTT_CLIENT_DATA_T *state, const char *name);                          // Tópico MQTT
void mqtt_setup(MQTT_CLIENT_DATA_T* state);

async_at_time_worker_t temp_and_humid_worker = { .do_work = worker_fn };

void vTaskMQTTClient(void *params)
{
    static MQTT_CLIENT_DATA_T state;    // Cria registro com os dados do cliente

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

    while (!state.connect_done || mqtt_client_is_connected(state.mqtt_client_inst)) {
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(10000));
    }
}

void mqtt_setup(MQTT_CLIENT_DATA_T* state) {
    char unique_id_buf[5];
    pico_get_unique_board_id_string(unique_id_buf, sizeof(unique_id_buf));
    for(int i=0; i < sizeof(unique_id_buf) - 1; i++) {
        unique_id_buf[i] = tolower(unique_id_buf[i]);
    }

    // Gera nome único, Ex: pico1234
    static char client_id_buf[sizeof(MQTT_DEVICE_NAME) + sizeof(unique_id_buf) - 1];
    memcpy(&client_id_buf[0], MQTT_DEVICE_NAME, sizeof(MQTT_DEVICE_NAME) - 1);
    memcpy(&client_id_buf[sizeof(MQTT_DEVICE_NAME) - 1], unique_id_buf, sizeof(unique_id_buf) - 1);
    client_id_buf[sizeof(client_id_buf) - 1] = 0;
    printf("Device name %s\n", client_id_buf);

    state->mqtt_client_info.client_id = client_id_buf;
    state->mqtt_client_info.keep_alive = MQTT_KEEP_ALIVE_S; // Keep alive in sec
#if defined(MQTT_USERNAME) && defined(MQTT_PASSWORD)
    state->mqtt_client_info.client_user = MQTT_USERNAME;
    state->mqtt_client_info.client_pass = MQTT_PASSWORD;
#else
    state->mqtt_client_info.client_user = NULL;
    state->mqtt_client_info.client_pass = NULL;
#endif
    static char will_topic[MQTT_TOPIC_LEN];
    strncpy(will_topic, full_topic(state, MQTT_WILL_TOPIC), sizeof(will_topic));
    state->mqtt_client_info.will_topic = will_topic;
    state->mqtt_client_info.will_msg = MQTT_WILL_MSG;
    state->mqtt_client_info.will_qos = MQTT_WILL_QOS;
    state->mqtt_client_info.will_retain = true;
#if LWIP_ALTCP && LWIP_ALTCP_TLS
    // TLS enabled
#ifdef MQTT_CERT_INC
    static const uint8_t ca_cert[] = TLS_ROOT_CERT;
    static const uint8_t client_key[] = TLS_CLIENT_KEY;
    static const uint8_t client_cert[] = TLS_CLIENT_CERT;
    // This confirms the indentity of the server and the client
    state->mqtt_client_info.tls_config = altcp_tls_create_config_client_2wayauth(ca_cert, sizeof(ca_cert),
            client_key, sizeof(client_key), NULL, 0, client_cert, sizeof(client_cert));
#if ALTCP_MBEDTLS_AUTHMODE != MBEDTLS_SSL_VERIFY_REQUIRED
    printf("Warning: tls without verification is insecure\n");
#endif
#else
    state->client_info.tls_config = altcp_tls_create_config_client(NULL, 0);
    printf("Warning: tls without a certificate is insecure\n");
#endif
#endif
}

// Controle do LED 
static void control(MQTT_CLIENT_DATA_T *state, bool on) {
    const char* message = on ? "On" : "Off";
    if (on)
        stop = true;
    else
        stop = false;

    mqtt_publish(state->mqtt_client_inst, full_topic(state, "/control/state"), message, strlen(message), MQTT_PUBLISH_QOS, MQTT_PUBLISH_RETAIN, pub_request_cb, state);
}

// Publicar temperatura
static void publish_temp_humid_and_lux(MQTT_CLIENT_DATA_T *state) {
    const char *temperature_key = full_topic(state, "/temperature");
    const char *humidity_key = full_topic(state, "/humidity");
    const char *lux_key = full_topic(state, "/luminosity");
    const char *quality_key = full_topic(state, "/quality");
    const char *status_key = full_topic(state, "/status");

    if(result!=DHT_RESULT_OK)
        return;

    char stt_str[16];
    snprintf(stt_str, sizeof(stt_str), (!stop)?"ATIVO":"PARADO");

    printf("Publishing %s to %s\n", stt_str, status_key);
    mqtt_publish(state->mqtt_client_inst, status_key, stt_str, strlen(stt_str), MQTT_PUBLISH_QOS, MQTT_PUBLISH_RETAIN, pub_request_cb, state);

    if(!stop) {
        bool status = ((quality >= 0.8) ? 0b010 : (quality < 0.8 && quality >= 0.5) ? 0b110 : 0b100);
        char temp_str[16], hum_str[16], lux_str[16], qlt_str[16];
        snprintf(temp_str, sizeof(temp_str), "%.2f", temperature);
        snprintf(hum_str, sizeof(hum_str), "%.2f", humidity);
        snprintf(lux_str, sizeof(lux_str), "%.2f", lux);
        snprintf(qlt_str, sizeof(qlt_str), (quality ==1)?"IDEAL":(quality >= 0.8)?"OTIMO":(quality >= 0.7)?"BOM":(quality >= 0.5)?"MEDIOCRE":"RUIM");

        printf("Publishing %s to %s\n", temp_str, temperature_key);
        mqtt_publish(state->mqtt_client_inst, temperature_key, temp_str, strlen(temp_str), MQTT_PUBLISH_QOS, MQTT_PUBLISH_RETAIN, pub_request_cb, state);

        printf("Publishing %s to %s\n", hum_str, humidity_key);
        mqtt_publish(state->mqtt_client_inst, humidity_key, hum_str, strlen(hum_str), MQTT_PUBLISH_QOS, MQTT_PUBLISH_RETAIN, pub_request_cb, state);

        printf("Publishing %s to %s\n", lux_str, lux_key);
        mqtt_publish(state->mqtt_client_inst, lux_key, lux_str, strlen(lux_str), MQTT_PUBLISH_QOS, MQTT_PUBLISH_RETAIN, pub_request_cb, state);

        printf("Publishing %s to %s\n", qlt_str, quality_key);
        mqtt_publish(state->mqtt_client_inst, quality_key, qlt_str, strlen(qlt_str), MQTT_PUBLISH_QOS, MQTT_PUBLISH_RETAIN, pub_request_cb, state);
    }
}

// Requisição para publicar
static void pub_request_cb(__unused void *arg, err_t err) {
    if (err != 0) {
        printf("pub_request_cb failed %d", err);
    }
}

// Requisição de Assinatura - subscribe
static void sub_request_cb(void *arg, err_t err) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    if (err != 0) {
        panic("subscribe request failed %d", err);
    }
    state->subscribe_count++;
}

// Requisição para encerrar a assinatura
static void unsub_request_cb(void *arg, err_t err) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    if (err != 0) {
        panic("unsubscribe request failed %d", err);
    }
    state->subscribe_count--;
    assert(state->subscribe_count >= 0);

    // Stop if requested
    if (state->subscribe_count <= 0 && state->stop_client) {
        mqtt_disconnect(state->mqtt_client_inst);
    }
}

// Tópicos de assinatura
static void sub_unsub_topics(MQTT_CLIENT_DATA_T* state, bool sub) {
    mqtt_request_cb_t cb = sub ? sub_request_cb : unsub_request_cb;
    mqtt_sub_unsub(state->mqtt_client_inst, full_topic(state, "/control"), MQTT_SUBSCRIBE_QOS, cb, state, sub);
}

// Dados de entrada MQTT
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
#if MQTT_UNIQUE_TOPIC
    const char *basic_topic = state->topic + strlen(state->mqtt_client_info.client_id) + 1;
#else
    const char *basic_topic = state->topic;
#endif
    strncpy(state->data, (const char *)data, len);
    state->len = len;
    state->data[len] = '\0';

    if (strcmp(basic_topic, "/control") == 0)
    {
        if (lwip_stricmp((const char *)state->data, "On") == 0 || strcmp((const char *)state->data, "1") == 0)
            control(state, true);
        else if (lwip_stricmp((const char *)state->data, "Off") == 0 || strcmp((const char *)state->data, "0") == 0)
            control(state, false);
    }
}

// Dados de entrada publicados
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    strncpy(state->topic, topic, sizeof(state->topic));
}

// Publicar temperatura
static void worker_fn(async_context_t *context, async_at_time_worker_t *worker) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)worker->user_data;
    publish_temp_humid_and_lux(state);
    async_context_add_at_time_worker_in_ms(context, worker, TEMP_WORKER_TIME_S * 1000);
}

// Conexão MQTT
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    if (status == MQTT_CONNECT_ACCEPTED) {
        state->connect_done = true;
        sub_unsub_topics(state, true);

        if (state->mqtt_client_info.will_topic) {
            mqtt_publish(state->mqtt_client_inst, state->mqtt_client_info.will_topic, "1", 1, MQTT_WILL_QOS, true, pub_request_cb, state);
        }

        temp_and_humid_worker.user_data = state;
        async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &temp_and_humid_worker, 0);
    } else if (status == MQTT_CONNECT_DISCONNECTED) {
        if (!state->connect_done) {
            panic("Failed to connect to mqtt server");
        }
    }
    else {
        panic("Unexpected status");
    }
}

// Inicializar o cliente MQTT
void start_client(MQTT_CLIENT_DATA_T *state) {
#if LWIP_ALTCP && LWIP_ALTCP_TLS
    const int port = MQTT_TLS_PORT;
    printf("Using TLS\n");
#else
    const int port = MQTT_PORT;
    printf("Warning: Not using TLS\n");
#endif

    state->mqtt_client_inst = mqtt_client_new();
    if (!state->mqtt_client_inst) {
        panic("MQTT client instance creation error");
    }
    printf("IP address of this device %s\n", ipaddr_ntoa(&(netif_list->ip_addr)));
    printf("Connecting to mqtt server at %s\n", ipaddr_ntoa(&state->mqtt_server_address));

    cyw43_arch_lwip_begin();
    if (mqtt_client_connect(state->mqtt_client_inst, &state->mqtt_server_address, port, mqtt_connection_cb, state, &state->mqtt_client_info) != ERR_OK) {
        panic("MQTT broker connection error");
    }
#if LWIP_ALTCP && LWIP_ALTCP_TLS
    // This is important for MBEDTLS_SSL_SERVER_NAME_INDICATION
    mbedtls_ssl_set_hostname(altcp_tls_context(state->mqtt_client_inst->conn), MQTT_SERVER);
#endif
    mqtt_set_inpub_callback(state->mqtt_client_inst, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, state);
    cyw43_arch_lwip_end();
}

// Call back com o resultado do DNS
void dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    MQTT_CLIENT_DATA_T *state = (MQTT_CLIENT_DATA_T*)arg;
    if (ipaddr) {
        state->mqtt_server_address = *ipaddr;
        start_client(state);
    } else {
        panic("dns request failed");
    }
}

//Topico MQTT
static const char *full_topic(MQTT_CLIENT_DATA_T *state, const char *name) {
#if MQTT_UNIQUE_TOPIC
    char full_topic[MQTT_TOPIC_LEN];
    snprintf(full_topic, sizeof(full_topic), "/%s%s", state->mqtt_client_info.client_id, name);
    return full_topic;
#else
    return name;
#endif
}