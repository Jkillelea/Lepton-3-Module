#ifndef SEGMENT_H_
#define SEGMENT_H_

#include "packet.h"

typedef struct segment {
    unsigned int segment_no;
    packet_t packets[60];
} segment_t;

void segment_append_packet(segment_t *seg, packet_t *pak);

#endif // SEGMENT_H_
