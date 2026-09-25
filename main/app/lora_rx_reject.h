#pragma once

#include <stdint.h>

#include "lora_packet.h"

void lora_rx_reject_record(
    lora_packet_reason_t reason,
    uint32_t device_id,
    uint8_t length,
    uint8_t expected_length);

void lora_rx_reject_record_crc(void);
void lora_rx_reject_record_length(uint8_t length, uint8_t expected_length);
void lora_rx_reject_record_device_id(uint32_t device_id);
void lora_rx_reject_record_parse(void);
