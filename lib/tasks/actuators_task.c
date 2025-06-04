#include "tasks.h"

void vTaskActuator(void *params)
{
    int status;

    while(true) {
        status = !stop*((temperature >= 25 && temperature <= 28) ? 0b00 : (temperature > 28) ? 0b01 : 0b10);

        gpio_put(HEATER, (status >> 1) & 1);
        gpio_put(REFRIGER, (status >> 0) & 1);

        status = !stop*((humidity >= 55 && humidity <= 70) ? 0b00 : (humidity > 70) ? 0b01 : 0b10);

        gpio_put(FAN_1, (status >> 1) & 1);
        gpio_put(FAN_2, (status >> 0) & 1);

        status = !stop*((lux >= 300 && lux <= 600) ? 0b00 : (lux > 600) ? 0b01 : 0b10);

        gpio_put(LIGHT, (status >> 1) & 1);
        gpio_put(CURTAIN, (status >> 0) & 1);

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}