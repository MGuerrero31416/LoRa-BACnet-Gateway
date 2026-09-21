set(APP_SRCS
    "main.c"

    "app/lora_bacnet_bridge.c"
    "app/lora_packet.c"

    "app/app_storage.c"
    "app/app_supervisor.c"
    "app/stack_profiler.c"

    "bacnet/bacnet_app.c"
    "bacnet/bacnet_coordinator.c"
    "bacnet/bacnet_event_bus.c"

    "bacnet/objects/analog_input.c"
    "bacnet/objects/analog_value.c"
    "bacnet/objects/binary_input.c"
    "bacnet/objects/binary_output.c"
    "bacnet/objects/binary_value.c"

    "platform/mstp_rs485.c"
    "platform/wifi_helper.c"

    "User_Settings.c"
)

if(CONFIG_USER_DISPLAY_LORA_GATEWAY)
    list(APPEND APP_SRCS
        "ui/profiles/display_lora_gateway.c"
        "app/lora_gateway.c"
        "platform/lora_hal.c"
        "platform/sx1262.c"
    )
elseif(NOT CMAKE_BUILD_EARLY_EXPANSION)
    message(FATAL_ERROR "No supported LoRa gateway profile selected")
endif()

set(REQUIRED_COMPONENTS
    bacnet-stack
    u8g2
    esp_driver_i2c
)

idf_component_register(
    SRCS
        ${APP_SRCS}

    INCLUDE_DIRS
        "."
        "app"
        "bacnet"
        "bacnet/objects"
        "platform"
        "ui"
        "ui/fonts"
        "ui/lvgl"

    REQUIRES
        ${REQUIRED_COMPONENTS}

    PRIV_REQUIRES
        spi_flash
        nvs_flash
        esp_event
        esp_wifi
        esp_netif
        driver
        esp_driver_gpio
        esp_driver_spi
        esp_driver_uart
        esp_timer
        mqtt
        mbedtls
)

# Add linker flags for Arduino log wrapper compatibility
target_link_options(
    ${COMPONENT_LIB}
    INTERFACE
        "-Wl,--wrap=esp_log_write"
        "-Wl,--wrap=esp_log_writev"
)

# Force the linker to include Arduino log wrapper symbols
target_link_libraries(
    ${COMPONENT_LIB}
    INTERFACE
        "-Wl,--undefined=__wrap_esp_log_write"
        "-Wl,--undefined=__wrap_esp_log_writev"
)

set_source_files_properties(
    "ui/profiles/display_st7796s.cpp"
    "ui/profiles/display_st7796s_test.cpp"
    "ui/profiles/display_lvgl_tdisplay_s3.cpp"
    PROPERTIES
        COMPILE_OPTIONS "-Wno-error=cpp;-Wno-error=deprecated-enum-enum-conversion"
)