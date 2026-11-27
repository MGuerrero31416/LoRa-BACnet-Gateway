#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef enum {
    RELIABILITY_NO_FAULT_DETECTED = 0,
    RELIABILITY_COMMUNICATION_FAILURE = 12
} BACNET_RELIABILITY;
void Analog_Input_Present_Value_Set(uint32_t instance, float value);
bool Analog_Input_Reliability_Set(uint32_t instance, BACNET_RELIABILITY value);
