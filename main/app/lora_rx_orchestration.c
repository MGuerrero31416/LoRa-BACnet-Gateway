#include "lora_rx_orchestration.h"

#include <string.h>

#include "lora_packet_validation.h"

bool lora_rx_orchestration_prepare_packet(
    const uint8_t *data,
    uint8_t length,
    uint32_t expected_version,
    uint32_t last_accepted_sequence,
    uint32_t *packet_device_id,
    lora_gateway_packet_data_t *packet,
    lora_packet_reason_t *reason)
{
    if (packet_device_id == NULL || packet == NULL || reason == NULL) {
        return false;
    }

    *packet_device_id = 0U;
    *reason = LORA_PACKET_ACCEPTED;
    memset(packet, 0, sizeof(*packet));

    if (data == NULL || length == 0U) {
        *reason = LORA_PACKET_REJECT_PARSE;
        return false;
    }

    memcpy(packet_device_id, &data[1], sizeof(*packet_device_id));
    if (*packet_device_id < LORA_DEVICE_ID_MIN || *packet_device_id > LORA_DEVICE_ID_MAX) {
        *reason = LORA_PACKET_REJECT_DEVICE_ID;
        return false;
    }

    return lora_packet_validation_decode(
        data,
        length,
        expected_version,
        last_accepted_sequence,
        packet,
        reason);
}
