#include "lora_rx_dispatch.h"

#include "esp_log.h"

#include "lora_bacnet_bridge.h"
#include "lora_gateway.h"
#include "lora_state_store.h"

#define TAG "lora_rx_dispatch"

bool lora_rx_dispatch_apply_packet(const lora_gateway_packet_data_t *packet)
{
    if (packet == NULL || !lora_device_id_valid(packet->device_id)) {
        return false;
    }

    if (!lora_state_store_update(packet->device_id, packet)) {
        return false;
    }

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

    return true;
}
