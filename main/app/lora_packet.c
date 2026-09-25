#include "lora_packet.h"

#include "lora_packet_validation.h"

bool lora_packet_decode(
    const uint8_t *packet,
    size_t packet_len,
    uint32_t expected_version,
    uint32_t last_accepted_sequence,
    lora_gateway_packet_data_t *decoded_packet,
    lora_packet_reason_t *reason)
{
    return lora_packet_validation_decode(
        packet,
        packet_len,
        expected_version,
        last_accepted_sequence,
        decoded_packet,
        reason);
}