#include "lora_rx_reject.h"

#include <inttypes.h>

#include "esp_log.h"

#define TAG "lora_rx_reject"

static uint32_t g_reject_counter[8] = {0U};

static void lora_rx_reject_incr(lora_packet_reason_t reason)
{
    if (reason < (sizeof(g_reject_counter) / sizeof(g_reject_counter[0]))) {
        g_reject_counter[reason]++;
    }
}

void lora_rx_reject_record(
    lora_packet_reason_t reason,
    uint32_t device_id,
    uint8_t length,
    uint8_t expected_length)
{
    lora_rx_reject_incr(reason);

    switch (reason) {
        case LORA_PACKET_REJECT_CRC:
            ESP_LOGW(TAG, "reject CRC");
            break;
        case LORA_PACKET_REJECT_LENGTH:
            ESP_LOGW(TAG, "reject length=%u expected=%u", length, expected_length);
            break;
        case LORA_PACKET_REJECT_DEVICE_ID:
            ESP_LOGW(TAG,
                     "reject device_id=%" PRIu32 " expected_range=%u..%u",
                     device_id,
                     LORA_DEVICE_ID_MIN,
                     LORA_DEVICE_ID_MAX);
            break;
        case LORA_PACKET_REJECT_PARSE:
            ESP_LOGW(TAG, "reject packet read");
            break;
        default:
            ESP_LOGW(TAG, "reject packet reason=%u", (unsigned)reason);
            break;
    }
}

void lora_rx_reject_record_crc(void)
{
    lora_rx_reject_record(LORA_PACKET_REJECT_CRC, 0U, 0U, 0U);
}

void lora_rx_reject_record_length(uint8_t length, uint8_t expected_length)
{
    lora_rx_reject_record(LORA_PACKET_REJECT_LENGTH, 0U, length, expected_length);
}

void lora_rx_reject_record_device_id(uint32_t device_id)
{
    lora_rx_reject_record(LORA_PACKET_REJECT_DEVICE_ID, device_id, 0U, 0U);
}

void lora_rx_reject_record_parse(void)
{
    lora_rx_reject_record(LORA_PACKET_REJECT_PARSE, 0U, 0U, 0U);
}
