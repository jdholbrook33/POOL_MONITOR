#include "flow.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"

#define TAG "FLOW_MONITOR"
#define PULSE_GPIO 2
#define PULSES_PER_GALLON 1140
#define HIGH_PULSE_THRESHOLD 300
#define CONSECUTIVE_HIGH_COUNT 5

static flow_data_t flow_data = {0};

void init_flow_meter(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1ULL << PULSE_GPIO,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PULSE_GPIO, pulse_isr_handler, NULL);
}

void IRAM_ATTR pulse_isr_handler(void *arg) {
    flow_data.pulse_count++;
    flow_data.pulse_rate++;
    flow_data.debug_pulse_count++;
}

bool validate_flow_data(flow_data_t *data) {
    data->data_valid = true;
    if (data->current_flow_rate > 17 || data->pulse_rate > 2000 || data->gallons_since_last > 1000 || data->total_gallons < flow_data.total_gallons) {
        data->data_valid = false;
        snprintf(data->error_msg, sizeof(data->error_msg), "Validation failed");
    }
    return data->data_valid;
}

char* create_flow_message(flow_data_t *data) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "device_id", "POOL_123");
    cJSON_AddNumberToObject(root, "timestamp", data->last_send_time);
    cJSON_AddNumberToObject(root, "total_gallons", data->total_gallons);
    cJSON_AddNumberToObject(root, "gallons_since_last", data->gallons_since_last);
    cJSON_AddNumberToObject(root, "flow_rate", data->current_flow_rate);
    cJSON_AddBoolToObject(root, "data_valid", data->data_valid);
    if (!data->data_valid) {
        cJSON_AddStringToObject(root, "error", data->error_msg);
    }
    char *json_string = cJSON_Print(root);
    cJSON_Delete(root);
    return json_string;
}

void flow_calc_task(void *pvParameter) {
    TickType_t last_wake_time = xTaskGetTickCount();
    while (1) {
        flow_data.current_flow_rate = (flow_data.pulse_rate * 60) / PULSES_PER_GALLON;
        uint32_t new_gallons = flow_data.pulse_count / PULSES_PER_GALLON;
        if (new_gallons > 0) {
            flow_data.total_gallons += new_gallons;
            flow_data.gallons_since_last += new_gallons;
            flow_data.pulse_count %= PULSES_PER_GALLON;
        }
        validate_flow_data(&flow_data);
        flow_data.pulse_rate = 0;
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(1000));
    }
}

void report_task(void *pvParameter) {
    TickType_t last_wake_time = xTaskGetTickCount();
    while (1) {
        validate_flow_data(&flow_data);
        char *message = create_flow_message(&flow_data);
        ESP_LOGI(TAG, "Flow Report: %s", message);
        free(message);
        flow_data.gallons_since_last = 0;
        flow_data.debug_pulse_count = 0;
        flow_data.last_send_time = time(NULL);
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(5 * 60 * 1000));
    }
}
