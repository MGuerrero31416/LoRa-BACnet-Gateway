#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "lora_gateway.h"
#include "lora_packet.h"

bool lora_state_store_init(void);
bool lora_state_store_get(uint32_t device_id, lora_device_state_t *device_state);
bool lora_state_store_update(uint32_t device_id, const lora_gateway_packet_data_t *packet);
size_t lora_state_store_get_dirty_device_ids(uint32_t *device_ids, size_t max_count);
void lora_state_store_clear_dirty(uint32_t device_id);
