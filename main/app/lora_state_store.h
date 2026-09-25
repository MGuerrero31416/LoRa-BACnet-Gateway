#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "lora_gateway.h"
#include "lora_packet.h"

bool lora_state_store_init(void);
bool lora_state_store_get(uint32_t device_id, lora_device_state_t *device_state);
bool lora_state_store_update(uint32_t device_id, const lora_gateway_packet_data_t *packet);
