#include "lora_bacnet_bridge.h"

#include "lora_gateway.h"
#include "User_Settings.h"

#include "bacnet/basic/object/ai.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LORA_BACNET_DEVICE_SLOT_COUNT 4U

static TaskHandle_t g_lora_bacnet_scheduler_task_handle = NULL;
static SemaphoreHandle_t g_lora_bacnet_publish_sem = NULL;

static const uint32_t g_lora_device_field_map[LORA_DEVICE_ID_MAX + 1U][LORA_BACNET_DEVICE_SLOT_COUNT] = {
    {0U, 0U, 0U, 0U},
    {0U, 1U, 2U, 3U},
    {4U, 5U, 6U, 7U},
    {8U, 9U, 10U, 11U},
    {12U, 13U, 14U, 15U},
    {16U, 17U, 18U, 19U},
    {20U, 21U, 22U, 23U}
};

static uint32_t lora_device_ai_for_field(uint32_t device_id, lora_bacnet_field_t field)
{
    if (!lora_device_id_valid(device_id)) {
        return 0U;
    }

    if (field >= LORA_BACNET_DEVICE_SLOT_COUNT) {
        return 0U;
    }

    const uint32_t slot = g_lora_device_field_map[device_id][field];
    return USER_AI_INSTANCES[slot];
}

static void lora_bacnet_publish_device_values(uint32_t device_id, const lora_device_state_t *device_state)
{
    if (device_state == NULL || !device_state->valid) {
        return;
    }

    const uint32_t ai_temperature = lora_device_ai_for_field(device_id, LORA_BACNET_FIELD_TEMP);
    const uint32_t ai_humidity = lora_device_ai_for_field(device_id, LORA_BACNET_FIELD_HUMIDITY);
    const uint32_t ai_voc = lora_device_ai_for_field(device_id, LORA_BACNET_FIELD_VOC);
    const uint32_t ai_pm25 = lora_device_ai_for_field(device_id, LORA_BACNET_FIELD_PM25);

    // Future LORA_STATUS: store this in a dedicated Multi-State Value object, not in an AI.
    const uint8_t lora_status_placeholder = LORA_STATUS_PLACEHOLDER_VALUE;
    (void)lora_status_placeholder;

    Analog_Input_Present_Value_Set(ai_temperature, device_state->temperature_c);
    Analog_Input_Present_Value_Set(ai_humidity, device_state->humidity_pct);
    Analog_Input_Present_Value_Set(ai_voc, device_state->voc_index);
    Analog_Input_Present_Value_Set(ai_pm25, device_state->pm2_5_ug_m3);
}

void lora_bacnet_publish_device(uint32_t device_id)
{
    lora_device_state_t device_state = {0};
    if (!lora_gateway_get_device_state(device_id, &device_state)) {
        return;
    }

    lora_bacnet_publish_device_values(device_id, &device_state);
}

void lora_bacnet_publish_all(void)
{
    for (uint32_t device_id = LORA_DEVICE_ID_MIN; device_id <= LORA_DEVICE_ID_MAX; ++device_id) {
        lora_bacnet_publish_device(device_id);
    }
}

void lora_bacnet_update_all_if_valid(void)
{
    for (uint32_t device_id = LORA_DEVICE_ID_MIN; device_id <= LORA_DEVICE_ID_MAX; ++device_id) {
        lora_bacnet_publish_device(device_id);
    }
}

void lora_bacnet_trigger_publish(void)
{
    if (g_lora_bacnet_publish_sem == NULL) {
        return;
    }

    xSemaphoreGive(g_lora_bacnet_publish_sem);
}

static void lora_bacnet_scheduler_task(void *argument)
{
    (void)argument;

    for (;;) {
        xSemaphoreTake(g_lora_bacnet_publish_sem, portMAX_DELAY);
        lora_bacnet_publish_all();
    }
}

esp_err_t lora_bacnet_scheduler_start(void)
{
    if (g_lora_bacnet_scheduler_task_handle != NULL) {
        return ESP_OK;
    }

    g_lora_bacnet_publish_sem = xSemaphoreCreateBinary();
    if (g_lora_bacnet_publish_sem == NULL) {
        return ESP_ERR_NO_MEM;
    }

    BaseType_t result = xTaskCreate(
        lora_bacnet_scheduler_task,
        "lora_bacnet_sched",
        3072,
        NULL,
        4,
        &g_lora_bacnet_scheduler_task_handle);

    if (result != pdPASS) {
        vSemaphoreDelete(g_lora_bacnet_publish_sem);
        g_lora_bacnet_publish_sem = NULL;
        g_lora_bacnet_scheduler_task_handle = NULL;
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}
