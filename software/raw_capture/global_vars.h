// part of raw_capture. Copyright 2019 Jacob Killelea <jkillelea@protonmail.ch>
#ifndef _RAW_CAPTURE_GLOBAL_VARS_H_
#define _RAW_CAPTURE_GLOBAL_VARS_H_

// Defines
#define IMAGE_WIDTH (160)
#define IMAGE_HEIGHT (120)
#define NUM_SEGMENTS (4)
#define PACKETS_PER_SEGMENT (60)
#define PACKETS_PER_FRAME (PACKETS_PER_SEGMENT*NUM_SEGMENTS)
#define PACKET_SIZE (164)
#define PACKET_SIZE_UINT16 (164/2)

extern uint16_t *image_ptr;

// I2C vars
extern uint16_t i2c_number;
extern LEP_CAMERA_PORT_DESC_T i2c_port;

// SPI protocol vars
extern int     spi_fd;
extern int     spi_speed;
extern uint8_t spi_mode;
extern char    spi_path[255];
extern uint8_t spi_bits_per_word;

// SPI communication vars
extern uint8_t packet[PACKET_SIZE];

#endif // _RAW_CAPTURE_GLOBAL_VARS_H_
