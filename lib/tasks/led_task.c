#include "tasks.h"

void vTaskLED(void *params)
{
    int status;
    while (true)
    {
        status = (quality >= 0.8) ? 0b010 : (quality < 0.8 && quality >= 0.5) ? 0b110 : 0b100;

        gpio_put(RED, (status >> 2) & 1);
        gpio_put(GREEN, (status >> 1) & 1);
        gpio_put(BLUE, (status >> 0) & 1);

        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
}