//Standard C library
#include <stddef.h>

//ESP-IDF error handling and logging
#include "esp_err.h"
#include "esp_log.h"

// FreeRTOS task types
// Required for TaskHandle_t variables used to track the BACnet and sensor tasks
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Application services
#include "app_storage.h"         /* Initialize NVS and persistence policy */
#include "app_supervisor.h"      /* Run periodic display/status maintenance */
#include "bacnet_app.h"          /* Initialize and start the BACnet runtime */
#include "display.h"             /* Initialize the physical display */
#include "lora_bacnet_bridge.h"  /* Publish LoRa-derived values into BACnet */
#include "lora_gateway.h"        /* Start LoRa receiver/gateway task */
#include "stack_profiler.h"      /* Monitor FreeRTOS task stack usage */

#include "User_Settings.h"

static const char *TAG = "bacnet";

static TaskHandle_t bacnet_rx_task_handle = NULL;
static TaskHandle_t bacnet_mstp_rx_task_handle = NULL;
static TaskHandle_t bacnet_core_task_handle = NULL;
static TaskHandle_t bacnet_cov_task_handle = NULL;
static TaskHandle_t lora_gateway_task_handle = NULL;

void app_main(void)
{
    // Bus recovery re-reserves pins it already owns, which the IDF gpio
    // reservation tracker can't tell apart from a real conflict; these two
    // tags only ever log that false positive at W, so keep errors visible
    // and drop the noise.
    esp_log_level_set("gpio_reserve", ESP_LOG_ERROR);
    esp_log_level_set("i2c.common", ESP_LOG_ERROR);

    ESP_ERROR_CHECK(app_storage_init());
    User_Settings_Print();

    const stack_profiler_task_handle_refs_t profiler_task_handles = {
        .bacnet_rx = &bacnet_rx_task_handle,
        .bacnet_mstp_rx = &bacnet_mstp_rx_task_handle,
        .bacnet_core = &bacnet_core_task_handle,
        .bacnet_cov = &bacnet_cov_task_handle,
        .sensor = NULL,
    };

    stack_profiler_init(&profiler_task_handles);

    ESP_ERROR_CHECK(
        bacnet_app_init(
            stack_profiler_bacnet_callback));

    ESP_LOGI(TAG, "Initializing display");
    display_init();
  

    /* Remaining task startup and supervisor loop */

    bacnet_app_task_handle_refs_t task_handles = {  // Provide task-handle references to the BACnet runtime
        .bip_rx = &bacnet_rx_task_handle,           // Receive task for BACnet/IP
        .mstp_rx = &bacnet_mstp_rx_task_handle,     // Receive task for BACnet MS/TP
        .core = &bacnet_core_task_handle,           // Core BACnet task for processing messages and events
        .cov = &bacnet_cov_task_handle,             // Change-of-Value (COV) notification task for BACnet
    };

    ESP_ERROR_CHECK(
        bacnet_app_start(&task_handles));           // Start BACnet runtime tasks

    ESP_ERROR_CHECK(
        lora_gateway_start(&lora_gateway_task_handle));
    ESP_ERROR_CHECK(
        lora_bacnet_scheduler_start());
    lora_bacnet_update_all_if_valid();

    app_supervisor_run();
}