# ST7796S Test UI

## Summary

Experimental UI for the same ESP32-S3 and ST7796S hardware used by the
[ST7796S colorful profile](st7796s-colorful.md).

Use this profile to test layout or drawing changes without modifying the main colorful UI.

## Selection

```text
ST7796S 480x320 - test UI
```

```c
CONFIG_USER_DISPLAY_ST7796S_TEST
```

## Hardware

The controller, resolution, wiring, SPI settings, backlight, and sensor defaults are identical to the
[ST7796S colorful profile](st7796s-colorful.md).

## Sensor defaults

The legacy auxiliary sensor defaults are not part of the active gateway profile documentation. This profile is retained only as a historical reference and does not represent the current release configuration.

## Source files

```text
main/ui/profiles/display_st7796s_test.cpp
components/TFT_eSPI/User_Setups/Setup_Project_ST7796S.h
```
