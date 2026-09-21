#pragma once

#include "esp_err.h"

#include <stdint.h>

#define LORA_BACNET_FIELD_TEMP_VALUE 0U
#define LORA_BACNET_FIELD_HUMIDITY_VALUE 1U
#define LORA_BACNET_FIELD_VOC_VALUE 2U
#define LORA_BACNET_FIELD_PM25_VALUE 3U

#define LORA_STATUS_PLACEHOLDER_VALUE 0U

typedef enum {
    LORA_BACNET_FIELD_TEMP = 0,
    LORA_BACNET_FIELD_HUMIDITY = 1,
    LORA_BACNET_FIELD_VOC = 2,
    LORA_BACNET_FIELD_PM25 = 3
} lora_bacnet_field_t;

void lora_bacnet_publish_device(uint32_t device_id);
void lora_bacnet_publish_all(void);
void lora_bacnet_update_all_if_valid(void);
void lora_bacnet_trigger_publish(void);
esp_err_t lora_bacnet_scheduler_start(void);

#define LORA_STATUS_MS_OBJECT_ID 400U
#define LORA_STATUS_MS_TEXT_COUNT 3U
#define LORA_STATUS_MS_STATE_OK 0U
#define LORA_STATUS_MS_STATE_WARNING 1U
#define LORA_STATUS_MS_STATE_ERROR 2U
