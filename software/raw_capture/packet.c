// part of raw_capture. Copyright 2019 Jacob Killelea <jkillelea@protonmail.ch>
#include "packet.h"
#include <string.h>

void packet_parse(uint8_t *raw_data, packet_t *packet) {
    packet->valid      = !((raw_data[0] & 0x0F) == 0x0F);
    packet->packet_no  = raw_data[1];

    if (packet->packet_no == 20) // only valid on packet 20
        packet->segment_no = raw_data[0] >> 4;
    else
        packet->segment_no = -1;

    for (uint32_t i = 0; i < 80; i++) {
        size_t idx = 2*i + 4;
        packet->data[i] = (raw_data[idx] << 8) | raw_data[idx+1];
    }
}
