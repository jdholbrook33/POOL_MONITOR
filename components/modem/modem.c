#include <string.h>
#include "modem.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/task.h"

#define TAG "MODEM"
#define UART_NUM UART_NUM_1
#define BUF_SIZE 1024

esp_err_t modem_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << NET_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure NET_PIN");
        return ret;
    }
    ESP_LOGI(TAG, "Modem GPIO initialized");
    return ESP_OK;
}

const char* modem_state_to_string(modem_state_t state) {
    switch(state) {
        case MODEM_OFF: return "OFF";
        case MODEM_BOOTING: return "BOOTING";
        case MODEM_CONNECTED: return "CONNECTED";
        default: return "UNKNOWN";
    }
}

modem_state_t check_modem_state(void) {
    int high_count = 0;
    for(int i = 0; i < LED_CHECK_SAMPLES; i++) {
        if(gpio_get_level(NET_PIN) == 1) {
            high_count++;
        }
        vTaskDelay(pdMS_TO_TICKS(LED_CHECK_INTERVAL_MS));
    }
    
    ESP_LOGI(TAG, "Sample high count: %d", high_count);

    if(high_count == LED_CHECK_SAMPLES) return MODEM_OFF;
    if(high_count == 0) return MODEM_BOOTING;
    return MODEM_CONNECTED;
}

esp_err_t monitor_modem_state_periodic(uint32_t duration_seconds) {
    ESP_LOGI(TAG, "Starting periodic state monitoring for %lu seconds", duration_seconds);
    
    uint32_t iterations = duration_seconds / 10;
    modem_state_t last_state = MODEM_OFF;
    
    for(uint32_t i = 0; i < iterations; i++) {
        modem_state_t current_state = check_modem_state();
        
        if(current_state != last_state) {
            ESP_LOGI(TAG, "State change detected!");
            ESP_LOGI(TAG, "Previous state: %s", modem_state_to_string(last_state));
            ESP_LOGI(TAG, "Current state:  %s", modem_state_to_string(current_state));
        } else {
            ESP_LOGI(TAG, "Current state: %s (t=%lu)", modem_state_to_string(current_state), i * 10);
        }
        
        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
    
    ESP_LOGI(TAG, "Periodic state monitoring complete");
    return ESP_OK;
}

void modem_monitor_task(void *pvParameters) {
    while(1) {
        modem_state_t current_state = check_modem_state();
        ESP_LOGI(TAG, "Modem State: %s", modem_state_to_string(current_state));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}