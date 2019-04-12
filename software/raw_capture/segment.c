#include "segment.h"
#include <stdint.h>
#include <string.h>

void segment_append_packet(segment_t *seg, packet_t *pak) {
    seg->packets[pak->packet_no] = *pak;
}
