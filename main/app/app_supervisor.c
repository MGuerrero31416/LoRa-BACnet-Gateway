#include "app_supervisor.h"

#include <stdint.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "User_Settings.h"
#include "bacnet_app.h"
#include "display.h"
#include "stack_profiler.h"

static const char *TAG = "app_supervisor";

void app_supervisor_run(void)
{
    uint32_t mstp_last_seen_pdu = 0;
    uint8_t mstp_alive_ticks = 0;

    if (USER_ENABLE_BACNET_MSTP) {
        ESP_LOGI(TAG, "BACnet MS/TP ready");
    }

    while (1) {
        /*
         * BACnet maintenance includes periodic MS/TP I-Am and
         * diagnostic counter handling.
         */
        bacnet_app_maintenance_1s();

        stack_profiler_sample(STACK_EVT_NORMAL);

        /*
         * Determine whether MS/TP traffic has recently been seen.
         */
        uint32_t mstp_pdu_count =
            bacnet_app_get_mstp_pdu_count();

        if (USER_ENABLE_BACNET_MSTP) {
            if (mstp_pdu_count != mstp_last_seen_pdu) {
                mstp_last_seen_pdu = mstp_pdu_count;
                mstp_alive_ticks = 6;
            } else if (mstp_alive_ticks > 0) {
                mstp_alive_ticks--;
            }
        } else {
            mstp_alive_ticks = 0;
        }

        display_set_link_status(
            bacnet_app_wifi_connected(),
            USER_ENABLE_BACNET_MSTP &&
                (mstp_alive_ticks > 0));

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}