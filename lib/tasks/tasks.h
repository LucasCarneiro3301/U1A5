#ifndef TASKS_H
#define TASKS_H

#include "stdio.h"
#include "hardware/pwm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <dht.h>
#include "../config/config.h"

#define MAX_USERS 25

extern SemaphoreHandle_t xDisplayMut;
extern SemaphoreHandle_t xStopSem;
extern ssd1306_t ssd;
extern dht_result_t result;
extern float temperature;
extern float humidity;
extern float lux;
extern float q_temp, q_humid, q_lux;
extern float quality;
extern bool stop;

// Frequências das notas musicais (em Hz)
enum Notes {
    DO = 2640, // Dó
    RE = 2970, // Ré
    MI = 3300, // Mi
    FA = 3520, // Fá
    SOL = 3960, // Sol
    LA = 4400, // Lá
    SI = 4950,  // Si
    DO_ALTO = 5280,  // Dó uma oitava acima (C5)
    LA_BAIXO = 880
};


void vTaskDisplay(void *params);
void vTaskDHT(void *params);
void vTaskMQTTClient(void *params);
void vTaskLDR(void *params);
void vTaskGMF(void *params);
void vTaskActuator(void *params);
void vTaskLED(void *params);

#endif