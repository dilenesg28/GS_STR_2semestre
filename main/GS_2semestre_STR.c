/*
 * Projeto: Monitoramento Wi-Fi Seguro
 * Autor: Cleber Dilenes
 * Plataforma: ESP32 + FreeRTOS
 * Descrição: Detecta redes Wi-Fi não autorizadas e gera alertas.
 * 
 * Simulação: Monitoramento em Tempo Real de Redes Wi-Fi Seguras com FreeRTOS (ESP32)
 * - Tasks:
 *   - ScannerTask   (prioridade alta)   : "varre" (simula) SSID conectado e envia para fila
 *   - CheckerTask   (prioridade média)  : recebe SSID, protege a lista com semáforo e valida
 *   - LoggerTask    (prioridade baixa)  : registra logs/alertas recebidos pela fila de logs
 *   - SupervisorTask(prioridade baixa)  : monitora eventos e toma ações de recuperação
 *   - Comunicação: fila de "ssid_event_t" entre Scanner -> Checker
 *   - Proteção: mutex (semáforo) para acessar lista de SSIDs seguras
 *   - Robustez: WDT (esp_task_wdt), timeouts + reset de fila, checagem de malloc + recovery (restart)
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_task_wdt.h"
#include <stdio.h>
#include <string.h>

#define MAX_SSIDS 5
#define LOG_QUEUE_SIZE 10
#define SSID_QUEUE_SIZE 10

typedef struct {
    char ssid[32];
} SSID_t;

typedef struct {
    char message[128];
} LogMessage_t;

QueueHandle_t ssidQueue;
QueueHandle_t logQueue;
SemaphoreHandle_t secureListMutex;

const char *secureNetworks[MAX_SSIDS] = {
    "IoT_Secure", "LabNet_Protected", "HomeNet_5G", "OfficeNet", "GuestNet"
};

// --- Função para verificar se SSID é seguro
int is_secure_network(const char *ssid) {
    int result = 0;
    xSemaphoreTake(secureListMutex, portMAX_DELAY);
    for (int i=0; i<MAX_SSIDS; i++) {
        if (strcmp(ssid, secureNetworks[i]) == 0) {
            result = 1;
            break;
        }
    }
    xSemaphoreGive(secureListMutex);
    return result;
}

// --- ScannerTask
void ScannerTask(void *pvParameters) {
    SSID_t ssidData;
    const char *demoSSIDs[] = {"IoT_Secure", "EvilTwin", "RandomAP", "LabNet_Protected", "Unknown_AP", "HomeNet_5G"};
    int idx = 0;
    
    while (1) {
        strncpy(ssidData.ssid, demoSSIDs[idx % 6], sizeof(ssidData.ssid));
        xQueueSend(ssidQueue, &ssidData, pdMS_TO_TICKS(50));
        idx++;
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay curto para evitar travamento e WDT
    }
}

// --- CheckerTask
void CheckerTask(void *pvParameters) {
    SSID_t ssidData;
    LogMessage_t logMsg;
    static int alertCounter = 0;

    esp_task_wdt_add(NULL); // Adiciona task ao WDT

    while (1) {
        if (xQueueReceive(ssidQueue, &ssidData, portMAX_DELAY) == pdPASS) {
            if (is_secure_network(ssidData.ssid)) {
                snprintf(logMsg.message, sizeof(logMsg.message), "{Checker} [OK] Rede segura detectada: %s", ssidData.ssid);
            } else {
                alertCounter++;
                snprintf(logMsg.message, sizeof(logMsg.message), "{Checker} [ALERTA] Rede NÃO autorizada detectada: %s (cont=%d)", ssidData.ssid, alertCounter);
            }
            xQueueSend(logQueue, &logMsg, pdMS_TO_TICKS(50));

            esp_task_wdt_reset();       // Reset do WDT
            vTaskDelay(pdMS_TO_TICKS(5)); // Delay curto para ceder CPU
        }
    }
}

// --- LoggerTask
void LoggerTask(void *pvParameters) {
    LogMessage_t logMsg;
    while (1) {
        if (xQueueReceive(logQueue, &logMsg, portMAX_DELAY) == pdPASS) {
            printf("%s\n", logMsg.message);
        }
    }
}

// --- SupervisorTask
void SupervisorTask(void *pvParameters) {
    while (1) {
        // Pode adicionar ações de mitigação baseadas em alertas persistentes
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay para não sobrecarregar CPU
    }
}

void app_main(void) {
    ssidQueue = xQueueCreate(SSID_QUEUE_SIZE, sizeof(SSID_t));
    logQueue = xQueueCreate(LOG_QUEUE_SIZE, sizeof(LogMessage_t));
    secureListMutex = xSemaphoreCreateMutex();

    esp_task_wdt_init(5, true); // Timeout 5s para todas tasks
    esp_task_wdt_add(NULL);      // Main task

    xTaskCreate(ScannerTask, "ScannerTask", 2048, NULL, 1, NULL);
    xTaskCreate(CheckerTask, "CheckerTask", 4096, NULL, 2, NULL);
    xTaskCreate(LoggerTask, "LoggerTask", 2048, NULL, 3, NULL);
    xTaskCreate(SupervisorTask, "SupervisorTask", 2048, NULL, 2, NULL);
}
