// A utility for capturing raw images from a FLIR Lepton 3.5
// call like: ./raw_capture 1 0 > image &&  ../../scripts/parse_txt_image.py image
// 'image' is a grid of numbers in text form
// returns 0 on success
// returns -1 if not all image segments are in order
// part of raw_capture. Copyright 2019 Jacob Killelea <jkillelea@protonmail.ch>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <LEPTON_SDK.h>
#include <LEPTON_OEM.h>
#include <LEPTON_SYS.h>
#include <LEPTON_RAD.h>
#include <LEPTON_Types.h>
#include <LEPTON_ErrorCodes.h>

#include "util.h"
#include "global_vars.h"
#include "packet.h"
#include "segment.h"

#define LOG(...)
// #define LOG(...) fprintf(stderr, __VA_ARGS__)

// I2C vars
uint16_t i2c_number = -1;
LEP_CAMERA_PORT_DESC_T i2c_port;

// SPI protocol vars
int     spi_fd    = -1;
int     spi_speed = 15000000;
uint8_t spi_mode  = SPI_MODE_3;
char    spi_path[255];
uint8_t spi_bits_per_word = 8;

// SPI communication vars
uint8_t spi_data[PACKET_SIZE] = {0x0};

// Image is made of 4 segments that are each made of 60 packets
segment_t segments[4];

void read_image() {
    uint32_t mismatches = 0; // number of times gotten out of sync
    segment_t segment;

    for (int32_t seg = 1; seg <= NUM_SEGMENTS; seg++) {
        for (int32_t pak = 0; pak < PACKETS_PER_SEGMENT; pak++) {
            packet_t *packet_ptr = &segment.packets[pak];

            if (read(spi_fd, spi_data, PACKET_SIZE) != PACKET_SIZE) // Read SPI
                LOG("SPI failed to read enough bytes!\n");

            // parse the bytes into a packet_t struct
            packet_parse(spi_data, packet_ptr); // NOTE: memory copy

            if (!packet_ptr->valid) { // handle drop packets
                LOG("drop (%x)\n", spi_data[0]);
                pak--;
                continue;
            }

            LOG("expected %d.%2d got %d.%2d\n", seg, pak,
                                                packet_ptr->segment_no,
                                                packet_ptr->packet_no);

            if (0 <= packet_ptr->packet_no && packet_ptr->packet_no < PACKETS_PER_SEGMENT)
                pak  = packet_ptr->packet_no;

            if (pak != packet_ptr->packet_no) { // out of sync
                LOG("mismatch %d\n", mismatches);
                pak = -1;
                mismatches++;
                usleep(1000);
                if (mismatches == 100) {
                    LOG("100 mismatches - resetting\n");
                    mismatches = 0;
                    close(spi_fd);
                    usleep(200*1000);
                    open_spi_port(spi_path);
                }
                continue;
            }

            // segment number is only valid on packet 20
            if (packet_ptr->packet_no == 20) {
                LOG("Expected segment %d, got %d\n", seg, packet_ptr->segment_no);
                if (packet_ptr->segment_no == 0) {
                    LOG("Invalid segment number. Going back to 1\n");
                    seg = 1;
                    continue;
                } else if (1 <= packet_ptr->segment_no
                             && packet_ptr->segment_no <= NUM_SEGMENTS) {
                    LOG("Resetting segment number.\n");
                    seg                = packet_ptr->segment_no;
                    segment.segment_no = packet_ptr->segment_no;
                }
            }

            segments[seg-1] = segment; // NOTE: memory copy
        }
        usleep(1000/106);
    }
}

int main(int argc, char *argv[]) {
    // setup
    // parse opts
    if (argc < 3) {
        fprintf(stderr, "Usage: %s i2c-number spi-number\n", argv[0]);
        return -1;
    }

    // I2C bus number
    switch (atoi(argv[1])) {
        case 0:
            i2c_number = 0;
            break;
        case 1:
            i2c_number = 1;
            break;
        default:
            pabort("Need to define I2C number as 0 or 1");
    }

    // Lepton config
    if (LEP_OpenPort(i2c_number, LEP_CCI_TWI, 400, &i2c_port) != LEP_OK)
        pabort("Couldn't open i2c port!");

    // Disable telemetry (changes packet lengths)
    if (LEP_SetSysTelemetryEnableState(&i2c_port, LEP_TELEMETRY_DISABLED) != LEP_OK)
        pabort("Couldn't disable telemetry!");

    // Enable radiometry
    if (LEP_SetRadEnableState(&i2c_port, LEP_RAD_ENABLE) != LEP_OK)
        pabort("Couldn't enable radiometry!");

    // SPI bus number
    if(set_spi_number(argv[2]) < 0)
        return -1;

    spi_fd = open_spi_port(spi_path);

    // Read the image
    for (uint32_t i = 0; i < 5; i++) {
        read_image();
    }

    LOG("Got segment numbers ");
    for (uint32_t seg = 0; seg < 4; seg++)
        LOG("%d ", segments[seg].segment_no);
    LOG("\n");

    // Do we have every segment in the right order
    bool transmission_ok = true;
    for (uint32_t seg = 0; seg < 4; seg++)
        transmission_ok = transmission_ok && (segments[seg].segment_no == seg+1);

    if (transmission_ok)
        fprintf(stderr, "OK\n");
    else
        fprintf(stderr, "ERROR\n");

    for (uint32_t seg = 0; seg < 4; seg++) {
        segment_t *segment = &segments[seg];
        for (uint32_t pak = 0; pak < 60; pak++) {
            packet_t *packet = &segment->packets[pak];
            for (uint32_t i = 0; i < 80; i++) {
                printf("%d ", packet->data[i]);
            }
            if (pak % 2 == 1)
                printf("\n");
        }
    }

    if (transmission_ok)
        return 0;
    return -1;
}


