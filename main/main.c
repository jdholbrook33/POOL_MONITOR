#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "flow.h"
#include "modem.h"
#include "certs.h"

void app_main(void) {
    // ESP_LOGI("MAIN", "Initializing Flow Meter");

    // // Initialize flow meter hardware and services
    // init_flow_meter();
    
    // Initialize the modem
    ESP_ERROR_CHECK(modem_init());

        // Create the monitoring task
    // xTaskCreate(modem_monitor_task, "modem_monitor", 2048, NULL, 5, NULL);
    
    // // Wait a bit then run the PWK test
    // vTaskDelay(pdMS_TO_TICKS(5000));  // Give monitoring task time to start
    // ESP_ERROR_CHECK(test_modem_pwk_cycle());

        ESP_ERROR_CHECK(monitor_modem_state_periodic(300));


    // Create tasks
    // xTaskCreate(flow_calc_task, "Flow Calc Task", 4096, NULL, 5, NULL);
    // xTaskCreate(report_task, "Report Task", 4096, NULL, 4, NULL);
    // xTaskCreate(modem_monitor_task, "modem_monitor", 2048, NULL, 5, NULL);
}
