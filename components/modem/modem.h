// modem.h
#ifndef MODEM_H
#define MODEM_H

#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// GPIO pins
#define TX_PIN 43
#define RX_PIN 44
#define PWK_PIN 4
#define RST_PIN 5
#define NET_PIN 6

// LED check parameters
#define LED_CHECK_INTERVAL_MS 25
#define LED_CHECK_SAMPLES 10

// Add this declaration
esp_err_t monitor_modem_state_periodic(uint32_t duration_seconds);

// Modem states 
typedef enum {
   MODEM_OFF,     // LED solid OFF (count = 0)
   MODEM_BOOTING, // LED solid ON (count = 10) 
   MODEM_CONNECTED // LED flashing (count between 1-9)
} modem_state_t;

// Function declarations
esp_err_t modem_init(void);
modem_state_t check_modem_state(void);
const char* modem_state_to_string(modem_state_t state);
void modem_monitor_task(void *pvParameters);

#endif // MODEM_H