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

#define LOG(...) fprintf(stderr, __VA_ARGS__)

// Image
uint16_t  image[IMAGE_HEIGHT][IMAGE_WIDTH];
uint16_t *image_ptr  = *image;

// I2C vars
uint16_t i2c_number = -1;
LEP_CAMERA_PORT_DESC_T i2c_port;

// SPI protocol vars
int     spi_fd    = -1;
int     spi_speed = 20000000;
uint8_t spi_mode  = SPI_MODE_3;
char    spi_path[255];
uint8_t spi_bits_per_word = 8;

// SPI communication vars
uint8_t packet[PACKET_SIZE] = {0x0};

inline static bool is_valid(uint8_t data) {
    return (data & 0x0f) == 0x0f;
}

void read_image(uint16_t *data_ptr) {
    static uint32_t mismatches = 0; // number of times gotten out of sync
    uint8_t segment_number     = 0; // segment number from SPI packet
    int16_t packet_number      = 0; // packet number from SPI packet

    for (int32_t seg = 1; seg <= NUM_SEGMENTS; seg++) {
        for (int32_t pak = 0; pak < PACKETS_PER_SEGMENT; pak++) {

            if (read(spi_fd, packet, PACKET_SIZE) != PACKET_SIZE) // Read SPI
                LOG("SPI failed to read enough bytes!\n");


            if (is_valid(packet[0])) { // handle drop packets
                LOG("drop (%x)\n", packet[0]);
                pak--;
                continue;
            }

            // get segment and packet number
            segment_number = (packet[0] >> 4);
            packet_number  = packet[1];
            LOG("expected packet %d.%2d got x.%2d\n", seg, pak, packet_number);

            if (0 <= packet[1] && packet[1] < PACKETS_PER_SEGMENT)
                pak  = packet[1];

            if (pak != packet_number) { // out of sync
                LOG("mismatch %d\n", mismatches);
                pak = -1;
                mismatches++;
                usleep(1000);
                if (mismatches == 100) {
                    mismatches = 0;
                    close(spi_fd);
                    usleep(200*1000);
                    open_spi_port(spi_path);
                }
                continue;
            }

            // segment number is only valid on packet 20
            if (pak == 20) {
                LOG("Expected segment %d, got %d\n", seg, segment_number);

                if (segment_number == 0) {
                    LOG("Invalid segment number. Going back to 1\n");
                    seg = 1;
                    continue;
                } else if (1 <= segment_number && segment_number <= NUM_SEGMENTS) {
                    LOG("Resetting segment number.\n");
                    seg = segment_number;
                }
            }

            // Copy the image data from the SPI packet
            // Can't use a straight memcpy since we have to account for endianness
            // when copying over. Lepton SPI is big endian and the pi's armv7l
            // processor is little endian
            size_t offset = 80*pak + 60*80*(seg-1);
            for (int i = 0; i < 80; i++) {
                size_t idx = 2*i + 4;
                data_ptr[offset + i] = packet[idx] << 8 | packet[idx + 1];
            }
        }
        usleep(1000/106);
    }
}

int main(int argc, char *argv[]) {
    // setup
    memset(image_ptr, 0, IMAGE_HEIGHT*IMAGE_WIDTH*sizeof(uint16_t));

    // parse opts
    if (argc < 3) {
        LOG("Usage: %s i2c-number spi-number\n", argv[0]);
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
    for (int i = 0; i < 1; i++) {
        read_image(image_ptr);
    }

    // Print to stdout
    for (int i = 0; i < IMAGE_HEIGHT; i++) {
        for (int j = 0; j < IMAGE_WIDTH; j++) {
            printf("%d ", image[i][j]);
        }
        printf("\n");
    }

    return 0;
}


