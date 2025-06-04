#include "tasks.h"
#include "math.h"

float temp_quality(float temperature) {
    float mu = 22.5;
    float sigma = 3.5;
    return exp(-pow((temperature - mu), 2) / (2 * pow(sigma, 2)));
}

float humid_quality(float humidity) {
    float mu = 62.5;
    float sigma = 10.0;
    return exp(-pow((humidity - mu), 2) / (2 * pow(sigma, 2)));
}

float lux_quality(float lux) {
    float mu = 450;
    float sigma = 150.0;
    return exp(-pow((lux - mu), 2) / (2 * pow(sigma, 2)));
}

void vTaskGMF(void *params)
{
    while (true)
    {
        q_temp = temp_quality(temperature);
        q_humid = humid_quality(humidity);
        q_lux = lux_quality(lux);

        quality = 0.4*q_temp + 0.4*q_humid + 0.2*q_lux;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}
