#include "lora_ui_result.h"

#include "display.h"

void lora_ui_result_handle(const lora_gateway_packet_data_t *packet)
{
    if (packet == NULL) {
        return;
    }

    display_update_lora_values(
        packet->voc_index,
        packet->pm2_5_ug_m3,
        packet->temperature_c,
        packet->humidity_pct,
        packet->device_id);
}
