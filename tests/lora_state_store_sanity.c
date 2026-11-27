#include "User_Settings.h"
#include "lora_state_store.h"

#include <assert.h>
#include <stdio.h>

static int64_t now_us;

int64_t esp_timer_get_time(void)
{
    return now_us;
}

bool lora_device_id_valid(uint32_t device_id)
{
    return device_id >= LORA_DEVICE_ID_MIN && device_id <= LORA_DEVICE_ID_MAX;
}

static size_t dirty_ids(uint32_t *ids)
{
    return lora_state_store_get_dirty_device_ids(ids, LORA_DEVICE_ID_MAX + 1U);
}

int main(void)
{
    const int64_t timeout_us = (int64_t)USER_LORA_TIMEOUT_SECONDS * 1000000LL;
    uint32_t ids[LORA_DEVICE_ID_MAX + 1U] = {0};
    lora_device_state_t state = {0};
    lora_gateway_packet_data_t packet = {
        .device_id = 1U,
        .temperature_c = 23.5f,
        .humidity_pct = 50.0f,
        .voc_index = 100.0f,
        .pm2_5_ug_m3 = 10.0f,
    };
    bool publish = false;

    assert(lora_state_store_init());
    lora_state_store_expire(timeout_us * 2);
    assert(dirty_ids(ids) == 0U);
    assert(lora_state_store_get(6U, &state) && !state.valid && !state.timed_out);

    assert(lora_state_store_update(1U, &packet, &publish) && publish);
    assert(dirty_ids(ids) == 1U && ids[0] == 1U);

    now_us = timeout_us - 1;
    lora_state_store_expire(now_us);
    assert(dirty_ids(ids) == 0U);
    assert(lora_state_store_update(1U, &packet, &publish) && !publish);
    now_us += 1;
    lora_state_store_expire(now_us);
    assert(dirty_ids(ids) == 0U);

    packet.device_id = 2U;
    assert(lora_state_store_update(2U, &packet, &publish) && publish);
    assert(dirty_ids(ids) == 1U && ids[0] == 2U);
    now_us = 2 * timeout_us - 2;
    lora_state_store_expire(now_us);
    assert(dirty_ids(ids) == 0U);
    now_us++;
    lora_state_store_expire(now_us);
    assert(dirty_ids(ids) == 1U && ids[0] == 1U);
    assert(lora_state_store_get(1U, &state) && state.valid && state.timed_out);
    assert(lora_state_store_get(2U, &state) && state.valid && !state.timed_out);
    lora_state_store_expire(now_us);
    assert(dirty_ids(ids) == 0U);

    packet.device_id = 1U;
    assert(lora_state_store_update(1U, &packet, &publish) && publish);
    assert(dirty_ids(ids) == 1U && ids[0] == 1U);
    assert(lora_state_store_get(1U, &state) && state.valid && !state.timed_out);
    assert(lora_state_store_update(1U, &packet, &publish) && !publish);
    puts("LoRa state store runtime sanity checks passed.");
    return 0;
}
