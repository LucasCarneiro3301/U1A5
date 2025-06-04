#include "tasks.h"

void vTaskDisplay(void *params)
{
    char str[32];

    while (true)
    {
        if(!stop) {
            ssd1306_fill(&ssd, false);                              // Limpa o display
            ssd1306_rect(&ssd, 1, 1, 123, 63, true, false);         // Retângulo da área útil
            sprintf(str, (temperature<100.0)?"TEMPC:%.2f C":"TEMPC:%.2fC", temperature);
            ssd1306_draw_string(&ssd, str ,4, 4);
            sprintf(str, (humidity<100.0)?"HUMID:%.2f %%":"HUMID:%.2f%%", humidity);
            ssd1306_draw_string(&ssd, str,4, 20);
            sprintf(str, (lux<100.0)?"LUMIN:%.2f lx":"LUMIN:%.2flx", lux);
            ssd1306_draw_string(&ssd, str,4, 36);
            sprintf(str, "QUALITY:%.2f", quality);
            ssd1306_draw_string(&ssd, str, 4, 52);
            //ssd1306_line(&ssd, 1, 36, 123, 36, true);		        // Desenha uma linha
            ssd1306_send_data(&ssd);                                // Atualiza o display
        } else {
            ssd1306_fill(&ssd, false);  		                // Limpa o display
            ssd1306_draw_string(&ssd, "STOP!!!", 28, 28);       // Desenha uma string
            ssd1306_send_data(&ssd);                            // Atualiza o display
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}