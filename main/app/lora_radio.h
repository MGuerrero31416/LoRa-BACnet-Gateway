#pragma once

#include "esp_err.h"
#include "platform/sx1262.h"

#include <stddef.h>
#include <stdint.h>

esp_err_t lora_radio_init(const lora_radio_config_t *config);
esp_err_t lora_radio_start_rx(void);
esp_err_t lora_radio_read_packet(uint8_t *buffer, size_t buffer_len, uint8_t *packet_len, uint16_t *irq_status);
