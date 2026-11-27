# BACnet Objects Configuration

The gateway creates 24 BACnet Analog Input application objects for LoRa
transmitter IDs 1–6.

## Object Counts

The counts are defined in `main/User_Settings.h`:

| Type | Created | Configured default slots |
|------|-------|---------------|
| Analog Values | 0 | `AV1`-`AV16` |
| Binary Values | 0 | `BV1`-`BV4` |
| Analog Inputs | 24 | `AI1`-`AI24` |
| Binary Inputs | 0 | `BI1`-`BI4` |
| Binary Outputs | 0 | `BO1`-`BO4` |

Only the Analog Input creation call is enabled in `main/bacnet/bacnet_app.c`.

## Configuration Sources

### Central User Configuration

`main/User_Settings.h` defines:

- object counts
- logical AI role identifiers
- logical BV role identifiers

`main/User_Settings.c` defines:

- BACnet instance arrays for every object type
- names
- descriptions
- units
- initial values
- active/inactive text for binary objects
- COV increments for analog objects

The object modules consume those arrays at runtime.

## Configurable Instance Arrays

The current code uses configurable BACnet instance arrays:

- `USER_AV_INSTANCES[USER_AV_COUNT]`
- `USER_BV_INSTANCES[USER_BV_COUNT]`
- `USER_AI_INSTANCES[USER_AI_COUNT]`
- `USER_BI_INSTANCES[USER_BI_COUNT]`
- `USER_BO_INSTANCES[USER_BO_COUNT]`

These arrays define the actual BACnet instance numbers exposed on the network.

That means:

- the first configured AI role does not have to remain BACnet instance `1`
- instance numbering can be changed without rewriting object-module logic
- logical sensor mapping stays stable even if BACnet instance numbers are reassigned

## Object Runtime Modules

The current project object implementations are in `main/bacnet/objects/`:

- `analog_value.c`
- `binary_value.c`
- `analog_input.c`
- `binary_input.c`
- `binary_output.c`

These modules:

- create project-specific BACnet objects
- apply defaults from `main/User_Settings.c`
- load saved state from NVS
- save writable properties back to NVS

## NVS Persistence

Object persistence is part of the current design.

The object modules store and restore data such as:

- object names
- object descriptions
- present values
- engineering units where supported
- COV increments for analog objects

Startup persistence policy is controlled by `main/app/app_storage.c` together with `USER_OVERRIDE_NVS_ON_FLASH` in `main/User_Settings.c`.

- `0`: preserve existing NVS data
- `1`: erase NVS on boot and restore compiled defaults

Saved names, descriptions, units, and COV increments override the compiled
defaults. For an existing installation, in particular, AI8 may retain the
former ID 2 dissolved-oxygen name, description, or units. Update those
properties through BACnet or deliberately reset the stored configuration
before relying on the new defaults. An NVS reset erases other stored settings
as well. Present values are reset to `-1` on startup, regardless of saved
values.

## Analog Input Mapping

All six IDs are accepted; each has four points in the same logical order.
IDs 1–4 use the SEN54 ranges and units. IDs 5–6 accept any finite values and
default to no units.

Default logical mapping:

| Logical slot | Default meaning |
|--------------|-----------------|
| AI1–AI4 | ID 1 temperature, humidity, VOC, PM2.5 |
| AI5–AI8 | ID 2 temperature, humidity, VOC, PM2.5 |
| AI9–AI12 | ID 3 temperature, humidity, VOC, PM2.5 |
| AI13–AI16 | ID 4 temperature, humidity, VOC, PM2.5 |
| AI17–AI20 | ID 5 generic inputs 1–4 (`05_AI1`–`05_AI4`) |
| AI21–AI24 | ID 6 generic inputs 1–4 (`06_AI1`–`06_AI4`) |

For generic transmitters, inputs 1–4 use the protocol's temperature,
humidity, VOC, and PM2.5 float fields, respectively. Their names do not change
the packet format. A transmitter must use a unique ID, compatible radio
settings and protocol version, and send valid packets.

## Missing Data

At startup each point is `-1` with no fault until its transmitter sends a
valid packet. Once seen, a transmitter that has not sent a valid packet for
`USER_LORA_TIMEOUT_SECONDS` (240 seconds by default, configurable in
`main/User_Settings.h`) has all four values set to `-1` and reliability set to
`communication-failure`; the BACnet fault status flag is then set. The timeout
is checked approximately once per second. The next valid packet restores the
measurements and clears the fault, even when the readings are unchanged.
After a receiver restart, transmitters are considered unseen again.

Configure the BACnet client to alert on reliability or the fault flag; these
properties alone do not guarantee an alarm notification. `-1` is not a unique
failure indicator, since it is a valid temperature and may also be a valid
generic input. The OLED displays only the latest accepted packet. If that
transmitter times out, its displayed values change to `-1` without displaying
an alert; the next accepted packet updates the display as usual.

The important distinction is:

- logical role order is fixed by the firmware
- BACnet instance numbers are supplied by the configurable instance arrays

## Where to Change Things

### Change object counts

Edit `main/User_Settings.h`.

Counts must remain consistent with the arrays and with any logic that assumes a minimum number of objects. The active gateway build keeps the basic AI/BV object set available for LoRa telemetry and status updates without relying on the retired auxiliary-sensor schema.

### Change BACnet instance numbers

Edit the `USER_*_INSTANCES[]` arrays in `main/User_Settings.c`.

### Change default names, descriptions, units, values, or COV increments

Edit the corresponding `USER_*` arrays in `main/User_Settings.c`.

### Change sensor-to-object mapping semantics

Edit the active LoRa-to-BACnet bridge implementation if you need to change
which logical AI role receives each LoRa measurement.

## Programmatic Updates

Typical runtime updates use the BACnet object APIs, for example:

```c
Analog_Value_Present_Value_Set(instance, value);
Analog_Input_Present_Value_Set(instance, value);
Binary_Value_Present_Value_Set(instance, BINARY_ACTIVE);
Binary_Input_Present_Value_Set(instance, BINARY_INACTIVE);
Binary_Output_Present_Value_Set(instance, BINARY_ACTIVE);
```

In the current application, `main/app/lora_bacnet_bridge.c` updates
LoRa-backed AIs using the configured instance numbers returned from
`USER_AI_INSTANCES[]`.

## Related Files

- `main/main.c`
- `main/User_Settings.h`
- `main/User_Settings.c`
- `main/app/app_storage.c`
- `main/app/lora_bacnet_bridge.c`
- `main/bacnet/bacnet_app.c`
- `main/bacnet/objects/analog_value.c`
- `main/bacnet/objects/binary_value.c`
- `main/bacnet/objects/analog_input.c`
- `main/bacnet/objects/binary_input.c`
- `main/bacnet/objects/binary_output.c`
