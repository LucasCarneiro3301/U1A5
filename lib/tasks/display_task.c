#include "tasks.h"

void vTaskDisplay(void *params)
{
    char str[32];

    while (true)
    {
        if ((xSemaphoreTake(xDisplayMut, portMAX_DELAY)==pdTRUE))
        {
            ssd1306_fill(&ssd, false);                              // Limpa o display
            ssd1306_rect(&ssd, 1, 1, 123, 63, true, false);         // Retângulo da área útil
            sprintf(str, "TEMPC:%.2f(C)", temperature);
            ssd1306_draw_string(&ssd, str ,4, 4);
            sprintf(str, "HUMID:%.2f(%%)", humidity);
            ssd1306_draw_string(&ssd, str,4, 20);
            sprintf(str, "LUMIN:%.2f(lx)", lux);
            ssd1306_draw_string(&ssd, str,4, 36);
            ssd1306_draw_string(&ssd, "QUALITY: -",4, 52);
            //ssd1306_line(&ssd, 1, 36, 123, 36, true);		        // Desenha uma linha
            ssd1306_send_data(&ssd);                                // Atualiza o display
            xSemaphoreGive(xDisplayMut);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}