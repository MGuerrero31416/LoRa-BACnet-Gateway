# Gateway profile

The active release build is the LoRa gateway profile only.

Select the processor with:

```text
ESP-IDF: Set Espressif Device Target
```

Keep the profile selection at:

```text
Project hardware
└── Display profile
    └── LoRa 32 V4 SX1262 gateway receiver
```

## Active profile

| Profile | Kconfig symbol | Intended target | Display | Interface |
|---|---|---|---|---|
| [Heltec WiFi LoRa 32 V4 gateway](lora-gateway.md) | `CONFIG_USER_DISPLAY_LORA_GATEWAY` | ESP32-S3 | SSD1315 OLED | I2C + SX1262 SPI |

This product branch intentionally omits the older non-gateway display and sensor profiles from the active configuration. The remaining hardware variation layer is limited to the LoRa gateway board and its display variant.
