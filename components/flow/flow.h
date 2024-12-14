#ifndef FLOW_H
#define FLOW_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// Data structure for flow meter data
typedef struct {
    uint32_t total_gallons;
    uint32_t gallons_since_last;
    uint32_t current_flow_rate;
    time_t last_send_time;
    volatile uint32_t pulse_count;
    volatile uint32_t pulse_rate;
    volatile uint32_t debug_pulse_count;
    bool data_valid;
    char error_msg[50];
} flow_data_t;

void init_flow_meter(void);
void flow_calc_task(void *pvParameter);
void report_task(void *pvParameter);
void pulse_isr_handler(void *arg);
bool validate_flow_data(flow_data_t *data);
char* create_flow_message(flow_data_t *data);

#endif // FLOW_H