// part of raw_capture. Copyright 2019 Jacob Killelea <jkillelea@protonmail.ch>
#ifndef PACKET_H_
#define PACKET_H_
#include <stdint.h>
#include <stdbool.h>

// TODO: memory alignment?
typedef struct packet {
    bool     valid;
    int32_t  segment_no; // -1 on all packets but number 20
    int32_t  packet_no;
    uint16_t data[80];
} packet_t;

void packet_parse(uint8_t *raw_data, packet_t *packet);

#endif // PACKET_H_
