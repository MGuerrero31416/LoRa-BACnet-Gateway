#include "User_Settings.h"
#include "lora_bacnet_bridge.h"
#include "lora_gateway.h"
#include "bacnet/basic/object/ai.h"

#include <assert.h>
#include <stdio.h>

const uint32_t USER_AI_INSTANCES[USER_AI_COUNT] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24
};
const float USER_AI_COV_INCREMENTS[USER_AI_COUNT] = {0};
static lora_device_state_t devices[LORA_DEVICE_ID_MAX + 1U];
static float values[USER_AI_COUNT];
static BACNET_RELIABILITY reliability[USER_AI_COUNT];

bool lora_device_id_valid(uint32_t id)
{
    return id >= LORA_DEVICE_ID_MIN && id <= LORA_DEVICE_ID_MAX;
}

bool lora_gateway_get_device_state(uint32_t id, lora_device_state_t *state)
{
    if (!lora_device_id_valid(id) || state == NULL) {
        return false;
    }
    *state = devices[id];
    return true;
}

void Analog_Input_Present_Value_Set(uint32_t instance, float value)
{
    assert(instance >= 1U && instance <= USER_AI_COUNT);
    values[instance - 1U] = value;
}

bool Analog_Input_Reliability_Set(uint32_t instance, BACNET_RELIABILITY value)
{
    assert(instance >= 1U && instance <= USER_AI_COUNT);
    reliability[instance - 1U] = value;
    return true;
}

bool app_storage_override_enabled(void) { return true; }
void bacnet_nvs_save_ai_pv(uint32_t instance, float value) { (void)instance; (void)value; }
int64_t esp_timer_get_time(void) { return 0; }
void lora_state_store_expire(int64_t now_us) { (void)now_us; }
void display_refresh_lora_timeout(void) {}
size_t lora_state_store_get_dirty_device_ids(uint32_t *ids, size_t count)
{
    (void)ids;
    (void)count;
    return 0;
}

int main(void)
{
    values[0] = -1.0f;
    lora_bacnet_publish_device(1U);
    assert(values[0] == -1.0f && reliability[0] == RELIABILITY_NO_FAULT_DETECTED);

    for (uint32_t id = 1U; id <= LORA_DEVICE_ID_MAX; ++id) {
        devices[id].valid = true;
        devices[id].temperature_c = (float)id * 10.0f + 1.0f;
        devices[id].humidity_pct = (float)id * 10.0f + 2.0f;
        devices[id].voc_index = (float)id * 10.0f + 3.0f;
        devices[id].pm2_5_ug_m3 = (float)id * 10.0f + 4.0f;
    }
    lora_bacnet_publish_all();
    for (uint32_t id = 1U; id <= LORA_DEVICE_ID_MAX; ++id) {
        for (uint32_t field = 0U; field < 4U; ++field) {
            const uint32_t slot = (id - 1U) * 4U + field;
            assert(values[slot] == (float)id * 10.0f + (float)field + 1.0f);
            assert(reliability[slot] == RELIABILITY_NO_FAULT_DETECTED);
        }
    }

    devices[2].timed_out = true;
    lora_bacnet_publish_device(2U);
    for (size_t slot = 4U; slot < 8U; ++slot) {
        assert(values[slot] == -1.0f);
        assert(reliability[slot] == RELIABILITY_COMMUNICATION_FAILURE);
    }
    assert(values[0] == 11.0f && values[8] == 31.0f);
    devices[2].timed_out = false;
    lora_bacnet_publish_device(2U);
    for (size_t slot = 4U; slot < 8U; ++slot) {
        assert(values[slot] == 21.0f + (float)(slot - 4U));
        assert(reliability[slot] == RELIABILITY_NO_FAULT_DETECTED);
    }
    puts("LoRa BACnet bridge runtime sanity checks passed.");
    return 0;
}
