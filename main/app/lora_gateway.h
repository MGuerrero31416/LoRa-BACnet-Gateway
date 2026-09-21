#pragma once

#include "lora_packet.h"

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t device_id;
    uint32_t last_sequence;
    bool valid;
    float temperature_c;
    float humidity_pct;
    float voc_index;
    float pm2_5_ug_m3;
    uint8_t status;
} lora_device_state_t;

bool lora_device_id_valid(uint32_t device_id);
bool lora_gateway_get_device_state(uint32_t device_id, lora_device_state_t *device_state);

esp_err_t lora_gateway_start(TaskHandle_t *task_handle);
