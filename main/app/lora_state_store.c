#include "lora_state_store.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static lora_device_state_t g_lora_devices[LORA_DEVICE_ID_MAX + 1U] = {0U};
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
    xSemaphoreGive(g_lora_devices_mutex);
    return true;
}
