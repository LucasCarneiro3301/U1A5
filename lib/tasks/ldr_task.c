#include "tasks.h"
#include "math.h"

#define R 10000.0
#define _3V3 3.3
#define RES 4095

uint16_t select_adc_channel(unsigned short int channel);

void vTaskLDR(void *params) 
{
    float rldr, vout;

    while(true) {
        uint16_t adc = select_adc_channel(2);

        vout = (adc * _3V3/RES);
        
        rldr = (R * (_3V3 - vout))/((vout)?vout:1e-3); 

        printf("RLDR: %f\n", rldr);

        lux = 12500 * pow(rldr, -0.7);  // Empiricamente

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Retorna a leitura de um determinado canal ADC
uint16_t select_adc_channel(unsigned short int channel) {
    adc_select_input(channel);
    return adc_read();
}