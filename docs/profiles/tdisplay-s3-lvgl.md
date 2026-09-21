# LilyGO T-Display-S3 LVGL

## Summary

ESP32-S3 profile for the LilyGO T-Display-S3 with an integrated 1.9-inch
ST7789 display, CST816 capacitive touch, and LVGL UI.

## Selection

```text
LVGL T-Display S3 - LilyGO LVGL UI
```

```c
CONFIG_USER_DISPLAY_LVGL_TDISPLAY_S3
```

This profile is available only when the ESP-IDF target is `esp32s3`.

## Source files

```text
main/ui/profiles/display_lvgl_tdisplay_s3.cpp
main/ui/hardware/touch_cst816.c
main/ui/hardware/touch_cst816.h
components/TFT_eSPI/User_Setups/Setup_Project_ST7789_TDISPLAY_S3.h
```

The display, touch, wiring, and target constraints are for the
LilyGO T-Display-S3 hardware. The profile uses the LVGL and ESP LVGL port
components for rendering.

The legacy auxiliary sensor defaults are intentionally not part of the current gateway documentation and should not be reintroduced into the release build.