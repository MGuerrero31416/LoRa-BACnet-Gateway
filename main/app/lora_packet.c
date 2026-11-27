#include "lora_packet.h"

#include "lora_packet_validation.h"

bool lora_packet_device_enabled(uint32_t device_id)
{
    static const uint32_t enabled_device_ids[] = {1U, 2U, 3U, 4U, 5U, 6U};

    for (size_t i = 0U; i < sizeof(enabled_device_ids) / sizeof(enabled_device_ids[0]); ++i) {
        if (enabled_device_ids[i] == device_id) {
            return true;
        }
    }

    return false;
}

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