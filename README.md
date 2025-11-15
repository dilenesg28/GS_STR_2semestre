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
 <img width="1020" height="620" alt="image" src="https://github.com/user-attachments/assets/57679479-952d-41ec-a4d4-8861f4ec6650" />
<img width="1117" height="632" alt="image" src="https://github.com/user-attachments/assets/79302210-699d-4971-929e-83b35b63ee20" />
