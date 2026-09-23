#ifndef DISPLAY_H
#define DISPLAY_H
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the display */
void display_init(void);

/* Update the gateway display with the latest received LoRa packet. */
void display_update_lora_values(
	float voc,
	float pm25,
	float temperature,
	float humidity,
	uint32_t device_id);

/* Update header link indicators (WiFi and MS/TP). */
void display_set_link_status(bool wifi_connected, bool mstp_connected);

/* Set display brightness in percent (0..100). */
void display_set_brightness(uint8_t percent);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_H */
