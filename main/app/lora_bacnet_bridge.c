#include "lora_bacnet_bridge.h"

#include "analog_input.h"
#include "app_storage.h"
#include "lora_gateway.h"
#include "User_Settings.h"

#include "bacnet/basic/object/ai.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <math.h>

#define LORA_BACNET_DEVICE_SLOT_COUNT 4U
#define LORA_BACNET_PERSIST_INTERVAL_MS 60000U

static TaskHandle_t g_lora_bacnet_scheduler_task_handle = NULL;
static SemaphoreHandle_t g_lora_bacnet_publish_sem = NULL;
static bool g_lora_bacnet_persist_valid[USER_AI_COUNT] = {0};
static float g_lora_bacnet_persisted_values[USER_AI_COUNT] = {0.0f};
static TickType_t g_lora_bacnet_persist_ticks[USER_AI_COUNT] = {0};

static const uint32_t g_lora_device_field_map[LORA_DEVICE_ID_MAX + 1U][LORA_BACNET_DEVICE_SLOT_COUNT] = {
    {0U, 0U, 0U, 0U},
    {0U, 1U, 2U, 3U},
    {4U, 5U, 6U, 7U},
    {8U, 9U, 10U, 11U},
    {12U, 13U, 14U, 15U}
    /* Provisionally disabled: LoRa transmitter IDs 5-6.
    {16U, 17U, 18U, 19U},
    {20U, 21U, 22U, 23U}
    */
};

static bool lora_device_ai_slot_for_field(uint32_t device_id, lora_bacnet_field_t field, uint32_t *slot)
{
    if (!lora_device_id_valid(device_id) || slot == NULL) {
        return false;
    }

    if (field >= LORA_BACNET_DEVICE_SLOT_COUNT) {
        return false;
    }

    *slot = g_lora_device_field_map[device_id][field];
    return *slot < USER_AI_COUNT;
}

static void lora_bacnet_publish_ai_value(uint32_t slot, float value)
{
    const uint32_t instance = USER_AI_INSTANCES[slot];

    Analog_Input_Present_Value_Set(instance, value);

    if (app_storage_override_enabled() || !isfinite(value)) {
        return;
    }

    const TickType_t now = xTaskGetTickCount();
    const TickType_t persist_interval_ticks =
        pdMS_TO_TICKS(LORA_BACNET_PERSIST_INTERVAL_MS);
    const float last_value = g_lora_bacnet_persisted_values[slot];
    const float cov_increment = USER_AI_COV_INCREMENTS[slot];
    const bool first_persist = !g_lora_bacnet_persist_valid[slot];
    const bool value_changed = fabsf(value - last_value) >= cov_increment;
    const bool interval_elapsed =
        (now - g_lora_bacnet_persist_ticks[slot]) >= persist_interval_ticks;

    if (first_persist || (value_changed && interval_elapsed)) {
        bacnet_nvs_save_ai_pv(instance, value);
        g_lora_bacnet_persisted_values[slot] = value;
        g_lora_bacnet_persist_ticks[slot] = now;
        g_lora_bacnet_persist_valid[slot] = true;
    }
}

static void lora_bacnet_publish_device_values(uint32_t device_id, const lora_device_state_t *device_state)
{
    if (device_state == NULL || !device_state->valid) {
        return;
    }

    uint32_t ai_temperature_slot = 0U;
    uint32_t ai_humidity_slot = 0U;
    uint32_t ai_voc_slot = 0U;
    uint32_t ai_pm25_slot = 0U;

    if (!lora_device_ai_slot_for_field(device_id, LORA_BACNET_FIELD_TEMP, &ai_temperature_slot) ||
        !lora_device_ai_slot_for_field(device_id, LORA_BACNET_FIELD_HUMIDITY, &ai_humidity_slot) ||
        !lora_device_ai_slot_for_field(device_id, LORA_BACNET_FIELD_VOC, &ai_voc_slot) ||
        !lora_device_ai_slot_for_field(device_id, LORA_BACNET_FIELD_PM25, &ai_pm25_slot)) {
        return;
    }

    // Future LORA_STATUS: store this in a dedicated Multi-State Value object, not in an AI.
    const uint8_t lora_status_placeholder = LORA_STATUS_PLACEHOLDER_VALUE;
    (void)lora_status_placeholder;

    lora_bacnet_publish_ai_value(ai_temperature_slot, device_state->temperature_c);
    lora_bacnet_publish_ai_value(ai_humidity_slot, device_state->humidity_pct);
    lora_bacnet_publish_ai_value(ai_voc_slot, device_state->voc_index);
    lora_bacnet_publish_ai_value(ai_pm25_slot, device_state->pm2_5_ug_m3);
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
