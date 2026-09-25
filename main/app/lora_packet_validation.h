#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lora_packet.h"

bool lora_packet_validation_decode(
    const uint8_t *packet,
    size_t packet_len,
    uint32_t expected_version,
    uint32_t last_accepted_sequence,
    lora_gateway_packet_data_t *decoded_packet,
    lora_packet_reason_t *reason);
