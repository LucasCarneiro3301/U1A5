#include "tasks.h"
#define DHT_PIN 16

void vTaskDHT(void *params)
{
    dht_t dht;
    dht_init(&dht, DHT11, pio0, DHT_PIN, true);

    while (true)
    {
        dht_start_measurement(&dht);

        result = dht_finish_measurement_blocking(&dht, &humidity, &temperature);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
