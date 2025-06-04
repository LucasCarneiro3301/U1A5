#include "tasks.h"

void jingle() {
    for (int i = 0; i < 2; i++) {
        play_buzzer(RE);
        vTaskDelay(pdMS_TO_TICKS(300));
        
        stop_buzzer();
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    stop_buzzer();
    vTaskDelay(pdMS_TO_TICKS(600));
}


void vTaskStatus(void *params)
{
    int status;
    while (true)
    {
        status = !stop*((quality >= 0.8) ? 0b010 : (quality < 0.8 && quality >= 0.5) ? 0b110 : 0b100);

        gpio_put(RED, (status >> 2) & 1);
        gpio_put(GREEN, (status >> 1) & 1);
        gpio_put(BLUE, stop);

        if(stop) {
            jingle();
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
}