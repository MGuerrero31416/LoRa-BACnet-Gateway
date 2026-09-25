#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "lora_packet.h"

bool lora_rx_dispatch_apply_packet(const lora_gateway_packet_data_t *packet);
