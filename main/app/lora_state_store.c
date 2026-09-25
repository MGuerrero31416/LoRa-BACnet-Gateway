#include "lora_state_store.h"

#include <stddef.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static lora_device_state_t g_lora_devices[LORA_DEVICE_ID_MAX + 1U] = {0U};
static bool g_lora_devices_dirty[LORA_DEVICE_ID_MAX + 1U] = {false};
static SemaphoreHandle_t g_lora_devices_mutex = NULL;

bool lora_state_store_init(void)
{
    if (g_lora_devices_mutex != NULL) {
        return true;
    }

    g_lora_devices_mutex = xSemaphoreCreateMutex();
    return g_lora_devices_mutex != NULL;
}

bool lora_state_store_get(uint32_t device_id, lora_device_state_t *device_state)
{
    if (!lora_device_id_valid(device_id) || device_state == NULL) {
        return false;
    }

    if (g_lora_devices_mutex == NULL && !lora_state_store_init()) {
        return false;
    }

    xSemaphoreTake(g_lora_devices_mutex, portMAX_DELAY);
    *device_state = g_lora_devices[device_id];
    xSemaphoreGive(g_lora_devices_mutex);
    return true;
}

bool lora_state_store_update(uint32_t device_id, const lora_gateway_packet_data_t *packet)
{
    if (!lora_device_id_valid(device_id) || packet == NULL) {
        return false;
    }

    if (g_lora_devices_mutex == NULL && !lora_state_store_init()) {
        return false;
    }

    xSemaphoreTake(g_lora_devices_mutex, portMAX_DELAY);
    lora_device_state_t *device_state = &g_lora_devices[device_id];
    device_state->device_id = packet->device_id;
    device_state->last_sequence = packet->sequence;
    device_state->temperature_c = packet->temperature_c;
    device_state->humidity_pct = packet->humidity_pct;
    device_state->voc_index = packet->voc_index;
    device_state->pm2_5_ug_m3 = packet->pm2_5_ug_m3;
    device_state->status = packet->status;
    device_state->valid = true;
    g_lora_devices_dirty[device_id] = true;
    xSemaphoreGive(g_lora_devices_mutex);
    return true;
}

size_t lora_state_store_get_dirty_device_ids(uint32_t *device_ids, size_t max_count)
{
    if (device_ids == NULL || max_count == 0U) {
        return 0U;
    }

    if (g_lora_devices_mutex == NULL && !lora_state_store_init()) {
        return 0U;
    }

    size_t count = 0U;
    xSemaphoreTake(g_lora_devices_mutex, portMAX_DELAY);
    for (uint32_t device_id = LORA_DEVICE_ID_MIN; device_id <= LORA_DEVICE_ID_MAX; ++device_id) {
        if (g_lora_devices_dirty[device_id]) {
            device_ids[count++] = device_id;
            g_lora_devices_dirty[device_id] = false;
            if (count >= max_count) {
                break;
            }
        }
    }
    xSemaphoreGive(g_lora_devices_mutex);
    return count;
}

void lora_state_store_clear_dirty(uint32_t device_id)
{
    if (!lora_device_id_valid(device_id)) {
        return;
    }

    if (g_lora_devices_mutex == NULL && !lora_state_store_init()) {
        return;
    }

    xSemaphoreTake(g_lora_devices_mutex, portMAX_DELAY);
    g_lora_devices_dirty[device_id] = false;
    xSemaphoreGive(g_lora_devices_mutex);
}
