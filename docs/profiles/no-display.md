# No Display

## Summary

Headless profile for running BACnet, sensors, persistence, and optional cloud publishing without a physical display.

## Selection

```text
No display
```

```c
CONFIG_USER_DISPLAY_NONE
```

## Supported targets

```text
esp32
esp32s3
```

The selected sensor GPIOs must still be valid for the physical board.

## Display implementation

The profile compiles no-operation implementations of the common display API:

```c
display_init();
display_update_values();
display_set_link_status();
```

## Sensor defaults

The legacy auxiliary sensor defaults are not part of the active gateway profile documentation. This profile does not compile the retired auxiliary-sensor application path.

## Source file

```text
main/ui/profiles/display_none.c
```

## Current TFT_eSPI build behavior

TFT_eSPI remains an unconditional component dependency. When the no-display profile is selected, `components/TFT_eSPI/User_Setup.h` includes a valid fallback setup so the library can compile.

The no-display implementation does not initialize or access a physical TFT.
