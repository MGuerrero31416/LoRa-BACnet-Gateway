#include "lora_packet.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void write_u32(uint8_t *destination, uint32_t value)
{
    memcpy(destination, &value, sizeof(value));
}

static void write_float(uint8_t *destination, float value)
{
    memcpy(destination, &value, sizeof(value));
}

static void make_packet(uint8_t packet[LORA_GATEWAY_PACKET_LEN], uint32_t device_id, uint32_t sequence)
{
    memset(packet, 0, LORA_GATEWAY_PACKET_LEN);
    packet[0] = LORA_GATEWAY_PROTOCOL_VERSION;
    write_u32(&packet[1], device_id);
    write_u32(&packet[5], sequence);
    write_float(&packet[9], 23.5F);
    write_float(&packet[13], 48.0F);
    write_float(&packet[17], 12.0F);
    write_float(&packet[21], 75.0F);
    packet[25] = LORA_GATEWAY_STATUS_OK;
}

static void expect_result(
    const char *name,
    const uint8_t *packet,
    size_t packet_len,
    uint32_t last_sequence,
    bool expected_accepted,
    lora_packet_reason_t expected_reason)
{
    lora_packet_reason_t reason = LORA_PACKET_ACCEPTED;
    lora_gateway_packet_data_t decoded_packet = {0};
    const bool accepted = lora_packet_decode(
        packet,
        packet_len,
        LORA_GATEWAY_PROTOCOL_VERSION,
        last_sequence,
        &decoded_packet,
        &reason);

    if (accepted != expected_accepted || reason != expected_reason) {
        fprintf(stderr, "FAIL %-28s accepted=%d reason=%d\n", name, accepted, reason);
        exit(EXIT_FAILURE);
    }

    if (accepted &&
        (decoded_packet.device_id == 0U ||
         (decoded_packet.sequence == 0U && last_sequence != UINT32_MAX))) {
        fprintf(stderr, "FAIL %-28s decoded packet fields invalid\n", name);
        exit(EXIT_FAILURE);
    }

    printf("PASS %s\n", name);
}

int main(void)
{
    uint8_t packet[LORA_GATEWAY_PACKET_LEN];

    for (uint32_t device_id = LORA_DEVICE_ID_MIN; device_id <= LORA_DEVICE_ID_MAX; ++device_id) {
        make_packet(packet, device_id, 11U);
        expect_result("valid sensor ID", packet, sizeof(packet), 10U, true, LORA_PACKET_ACCEPTED);
    }

    expect_result("wrong length", packet, sizeof(packet) - 1U, 10U, false, LORA_PACKET_REJECT_LENGTH);

    packet[0] = 2U;
    expect_result("wrong version", packet, sizeof(packet), 10U, false, LORA_PACKET_REJECT_VERSION);

    make_packet(packet, 0U, 11U);
    expect_result("device below range", packet, sizeof(packet), 10U, false, LORA_PACKET_REJECT_DEVICE_ID);

    make_packet(packet, LORA_DEVICE_ID_MAX + 1U, 11U);
    expect_result("device above range", packet, sizeof(packet), 10U, false, LORA_PACKET_REJECT_DEVICE_ID);

    make_packet(packet, 1U, 10U);
    expect_result("duplicate sequence accepted", packet, sizeof(packet), 10U, true, LORA_PACKET_ACCEPTED);

    make_packet(packet, 1U, 9U);
    expect_result("older sequence accepted", packet, sizeof(packet), 10U, true, LORA_PACKET_ACCEPTED);

    make_packet(packet, 1U, 0U);
    expect_result("sequence wraparound", packet, sizeof(packet), UINT32_MAX, true, LORA_PACKET_ACCEPTED);

    make_packet(packet, 1U, 10U);
    packet[25] = LORA_GATEWAY_STATUS_WARNING;
    expect_result("warning with older sequence", packet, sizeof(packet), 10U, true, LORA_PACKET_ACCEPTED);

    make_packet(packet, 1U, 11U);
    write_float(&packet[9], NAN);
    expect_result("non-finite sensor value", packet, sizeof(packet), 10U, false, LORA_PACKET_REJECT_RANGE);

    expect_result("null packet", NULL, 0U, 10U, false, LORA_PACKET_REJECT_PARSE);

    puts("LoRa packet runtime sanity checks passed.");
    return EXIT_SUCCESS;
}