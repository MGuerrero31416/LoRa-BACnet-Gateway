#include "lora_radio.h"

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "platform/lora_hal.h"
#include "platform/sx1262.h"

#include <string.h>

#define TAG "lora_radio"

esp_err_t lora_radio_init(const lora_radio_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    return sx1262_configure(config);
}

esp_err_t lora_radio_start_rx(void)
{
    lora_hal_fem_set_rx();
    lora_hal_clear_event();

    esp_err_t result = sx1262_set_irq_mask(SX1262_IRQ_RX_EVENTS);
    if (result == ESP_OK) {
        result = sx1262_clear_irq();
    }
    if (result == ESP_OK) {
        result = sx1262_start_rx_continuous();
    }

    return result;
}

esp_err_t lora_radio_read_packet(uint8_t *buffer, size_t buffer_len, uint8_t *packet_len, uint16_t *irq_status)
{
    if (buffer == NULL || packet_len == NULL || irq_status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!lora_hal_wait_event(portMAX_DELAY)) {
        return ESP_ERR_TIMEOUT;
    }

    uint8_t length = 0U;
    uint8_t offset = 0U;
    if (sx1262_get_irq_status(irq_status) != ESP_OK || sx1262_clear_irq() != ESP_OK) {
        ESP_LOGW(TAG, "radio IRQ read failed");
        return ESP_FAIL;
    }

    if ((*irq_status & SX1262_IRQ_CRC_ERROR) != 0U) {
        return ESP_ERR_INVALID_CRC;
    }

    if ((*irq_status & SX1262_IRQ_RX_DONE) == 0U) {
        return ESP_ERR_TIMEOUT;
    }

    if (sx1262_get_rx_buffer_status(&length, &offset) != ESP_OK) {
        return ESP_FAIL;
    }

    if (length == 0U || length > buffer_len) {
        return ESP_ERR_INVALID_SIZE;
    }

    memset(buffer, 0, buffer_len);
    if (sx1262_read_buffer(offset, buffer, length) != ESP_OK) {
        return ESP_FAIL;
    }

    *packet_len = length;
    return ESP_OK;
}
