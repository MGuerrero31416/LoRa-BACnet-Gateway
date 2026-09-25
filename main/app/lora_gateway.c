/*
 * LoRa packet format (little-endian, packed, 26 bytes):
 *   0: VERSION (uint8_t)
 *   1-4: DEVICE_ID (uint32_t)
 *   5-8: SEQUENCE (uint32_t)
 *   9-12: TEMPERATURE (float IEEE-754)
 *   13-16: HUMIDITY (float IEEE-754)
 *   17-20: PM2_5 (float IEEE-754)
 *   21-24: VOC (float IEEE-754)
 *   25: STATUS (uint8_t)
 *
 * Validation rules:
 *   - CRC must pass before radio receive callback is accepted;
 *   - length must be exactly 26 bytes;
 *   - version must match the supported gateway protocol version;
 *   - device id must match the configured gateway sensor ID;
 *   - sequence is decoded and retained with the sensor state, but does not
 *     restrict acceptance of periodic telemetry packets;
 *   - temperature, humidity, PM2.5, and VOC values must be in sane ranges.
 */
#include "lora_gateway.h"

#include "lora_bacnet_bridge.h"
#include "lora_packet_validation.h"
#include "lora_radio.h"
#include "lora_rx_orchestration.h"
#include "lora_state_store.h"
#include "User_Settings.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "platform/lora_hal.h"
#include "platform/sx1262.h"
#include "display.h"

#include "bacnet/basic/object/ai.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define TAG "lora_gateway"

static const lora_radio_config_t g_lora_radio = {
    .frequency_hz = 923000000UL,
    .bandwidth = 0x04,
    .spreading_factor = 10,
    .coding_rate = 0x01,
    .preamble_length = 12,
    .tx_power_dbm = 22,
    .packet_max_len = LORA_PACKET_MAX_LEN,
};

static uint32_t g_reject_counter[8] = {0U};

bool lora_device_id_valid(uint32_t device_id)
{
    return device_id >= LORA_DEVICE_ID_MIN && device_id <= LORA_DEVICE_ID_MAX;
}

bool lora_gateway_get_device_state(uint32_t device_id, lora_device_state_t *device_state)
{
    return lora_state_store_get(device_id, device_state);
}

static void lora_gateway_publish_valid_packet(const lora_gateway_packet_data_t *packet)
{
    if (packet == NULL || !lora_device_id_valid(packet->device_id)) {
        return;
    }

    lora_state_store_update(packet->device_id, packet);

    lora_bacnet_trigger_publish();

    ESP_LOGI(TAG,
             "accepted LoRa packet device=%" PRIu32 " seq=%" PRIu32 " temp=%.2f RH=%.2f PM2.5=%.2f VOC=%.2f status=%u",
             packet->device_id,
             packet->sequence,
             packet->temperature_c,
             packet->humidity_pct,
             packet->pm2_5_ug_m3,
             packet->voc_index,
             packet->status);
}

static void lora_gateway_task(void *argument)
{
    (void)argument;
    ESP_LOGI(TAG, "LoRa gateway task started");

    const uint32_t expected_version = LORA_GATEWAY_PROTOCOL_VERSION;

    uint8_t data[LORA_GATEWAY_PACKET_LEN] = {0};
    bool receive_setup_error_logged = false;

    if (lora_radio_init(&g_lora_radio) != ESP_OK) {
        ESP_LOGE(TAG, "SX1262 configure failed");
        vTaskDelete(NULL);
    }

    for (;;) {
        esp_err_t receive_setup_result = lora_radio_start_rx();
        if (receive_setup_result != ESP_OK) {
            if (!receive_setup_error_logged) {
                ESP_LOGE(TAG, "radio receive setup failed: %s", esp_err_to_name(receive_setup_result));
                receive_setup_error_logged = true;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        if (receive_setup_error_logged) {
            ESP_LOGI(TAG, "radio receive setup recovered");
            receive_setup_error_logged = false;
        }

        uint16_t irq_status = 0U;
        uint8_t length = 0U;
        esp_err_t read_result = lora_radio_read_packet(data, sizeof(data), &length, &irq_status);
        if (read_result == ESP_ERR_INVALID_CRC) {
            g_reject_counter[LORA_PACKET_REJECT_CRC]++;
            ESP_LOGW(TAG, "reject CRC");
            continue;
        }
        if (read_result == ESP_ERR_INVALID_SIZE) {
            g_reject_counter[LORA_PACKET_REJECT_LENGTH]++;
            ESP_LOGW(TAG, "reject length=%u expected=%u", length, LORA_GATEWAY_PACKET_LEN);
            continue;
        }
        if (read_result == ESP_ERR_TIMEOUT) {
            continue;
        }
        if (read_result != ESP_OK) {
            g_reject_counter[LORA_PACKET_REJECT_PARSE]++;
            ESP_LOGW(TAG, "reject packet read");
            continue;
        }

        if (length != LORA_GATEWAY_PACKET_LEN) {
            g_reject_counter[LORA_PACKET_REJECT_LENGTH]++;
            ESP_LOGW(TAG, "reject length=%u expected=%u", length, LORA_GATEWAY_PACKET_LEN);
            continue;
        }

        uint32_t packet_device_id = 0U;
        memcpy(&packet_device_id, &data[1], sizeof(packet_device_id));
        if (!lora_device_id_valid(packet_device_id)) {
            g_reject_counter[LORA_PACKET_REJECT_DEVICE_ID]++;
            ESP_LOGW(TAG,
                     "reject device_id=%" PRIu32 " expected_range=%u..%u",
                     packet_device_id,
                     LORA_DEVICE_ID_MIN,
                     LORA_DEVICE_ID_MAX);
            continue;
        }

        uint32_t packet_sequence = 0U;
        memcpy(&packet_sequence, &data[5], sizeof(packet_sequence));
        lora_device_state_t device_state = {0};
        if (!lora_gateway_get_device_state(packet_device_id, &device_state)) {
            ESP_LOGE(TAG, "device state unavailable");
            continue;
        }

        const uint32_t last_accepted_sequence =
            device_state.valid ? device_state.last_sequence : packet_sequence - 1U;
        lora_packet_reason_t reason = LORA_PACKET_ACCEPTED;
        lora_gateway_packet_data_t packet = {0};
        if (!lora_rx_orchestration_prepare_packet(
                data,
                length,
                expected_version,
                last_accepted_sequence,
                &packet_device_id,
                &packet,
                &reason)) {
            if (reason < (sizeof(g_reject_counter) / sizeof(g_reject_counter[0]))) {
                g_reject_counter[reason]++;
            }
            ESP_LOGW(TAG, "reject packet reason=%u", (unsigned)reason);
            continue;
        }

        lora_gateway_publish_valid_packet(&packet);
        display_update_lora_values(
            packet.voc_index,
            packet.pm2_5_ug_m3,
            packet.temperature_c,
            packet.humidity_pct,
            packet.device_id);
    }
}

esp_err_t lora_gateway_start(TaskHandle_t *task_handle)
{
    if (task_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_RETURN_ON_ERROR(lora_hal_init(), TAG, "LoRa HAL init failed");

    if (!lora_state_store_init()) {
        return ESP_ERR_NO_MEM;
    }

    BaseType_t result = xTaskCreate(lora_gateway_task, "lora_gateway", 4096, NULL, 4, task_handle);
    if (result != pdPASS) {
        *task_handle = NULL;
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}
