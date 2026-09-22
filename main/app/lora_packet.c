#include "lora_packet.h"

#include <math.h>
#include <string.h>

static bool lora_range_valid(float value, float min, float max)
{
    return isfinite(value) && value >= min && value <= max;
}

bool lora_packet_decode(
    const uint8_t *packet,
    size_t packet_len,
    uint32_t expected_version,
    uint32_t last_accepted_sequence,
    lora_gateway_packet_data_t *decoded_packet,
    lora_packet_reason_t *reason)
{
    if (reason == NULL || decoded_packet == NULL) {
        return false;
    }

    (void)last_accepted_sequence;
    *reason = LORA_PACKET_ACCEPTED;
    memset(decoded_packet, 0, sizeof(*decoded_packet));
    if (packet == NULL || packet_len == 0U) {
        *reason = LORA_PACKET_REJECT_PARSE;
        return false;
    }

    if (packet_len != LORA_GATEWAY_PACKET_LEN) {
        *reason = LORA_PACKET_REJECT_LENGTH;
        return false;
    }

    if ((uint32_t)packet[0] != expected_version) {
        *reason = LORA_PACKET_REJECT_VERSION;
        return false;
    }

    uint32_t device_id = 0U;
    memcpy(&device_id, &packet[1], sizeof(device_id));
    if (device_id < LORA_DEVICE_ID_MIN || device_id > LORA_DEVICE_ID_MAX) {
        *reason = LORA_PACKET_REJECT_DEVICE_ID;
        return false;
    }

    float temperature = 0.0F;
    float humidity = 0.0F;
    float pm2_5 = 0.0F;
    float voc = 0.0F;
    memcpy(&temperature, &packet[9], sizeof(temperature));
    memcpy(&humidity, &packet[13], sizeof(humidity));
    memcpy(&pm2_5, &packet[17], sizeof(pm2_5));
    memcpy(&voc, &packet[21], sizeof(voc));
    if (!lora_range_valid(temperature, -60.0F, 150.0F) ||
        !lora_range_valid(humidity, 0.0F, 100.0F) ||
        !lora_range_valid(pm2_5, 0.0F, 1000.0F) ||
        !lora_range_valid(voc, 0.0F, 5000.0F)) {
        *reason = LORA_PACKET_REJECT_RANGE;
        return false;
    }

    uint32_t sequence = 0U;
    memcpy(&sequence, &packet[5], sizeof(sequence));

    decoded_packet->device_id = device_id;
    decoded_packet->sequence = sequence;
    decoded_packet->temperature_c = temperature;
    decoded_packet->humidity_pct = humidity;
    decoded_packet->pm2_5_ug_m3 = pm2_5;
    decoded_packet->voc_index = voc;
    decoded_packet->status = packet[25];
    return true;
}