#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "lora_packet.h"

bool lora_rx_orchestration_prepare_packet(
    const uint8_t *data,
    uint8_t length,
    uint32_t expected_version,
    uint32_t last_accepted_sequence,
    uint32_t *packet_device_id,
    lora_gateway_packet_data_t *packet,
    lora_packet_reason_t *reason);
