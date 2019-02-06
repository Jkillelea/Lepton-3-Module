#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
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

#define IMAGE_WIDTH (160)
#define IMAGE_HEIGHT (120)
#define NUM_SEGMENTS (4)
#define PACKETS_PER_SEGMENT (60)
#define PACKETS_PER_FRAME (PACKETS_PER_SEGMENT*NUM_SEGMENTS)
#define PACKET_SIZE (164)
#define PACKET_SIZE_UINT16 (164/2)

// Image
static uint16_t  image[IMAGE_HEIGHT][IMAGE_WIDTH];
static uint16_t *image_ptr  = *image;

// I2C vars
static uint16_t i2c_number = -1;
static LEP_CAMERA_PORT_DESC_T i2c_port;

// SPI protocol vars
static int     spi_fd    = -1;
static int     spi_speed = 20000000;
static uint8_t spi_mode  = SPI_MODE_3;
static char    spi_path[255];
static uint8_t spi_bits_per_word = 8;

// SPI communication vars
static uint8_t packet[PACKET_SIZE] = {0x0};

// Functions
static void pabort(const char *s);
static int open_spi_port(const char *path);
static int set_spi_number(const char *arg);

void read_image(uint16_t *data_ptr) {
    static uint32_t mismatches = 0; // number of times gotten out of sync
    uint8_t segment_number     = 0; // segment number from SPI packet
    int16_t packet_number      = 0; // packet number from SPI packet
    int32_t seg, pak           = 0; // loop vars

    for (seg = 1; seg <= NUM_SEGMENTS; seg++) {
        for (pak = 0; pak < PACKETS_PER_SEGMENT; pak++) {
            // Read SPI
            if (read(spi_fd, packet, PACKET_SIZE) != PACKET_SIZE)
                fprintf(stderr, "SPI failed to read enough bytes!\n");

            // handle drop packets
            if ((packet[0] & 0x0f) == 0x0f) {
                fprintf(stderr, "drop (%x)\n", packet[0]);
                pak--;
                continue;
            } 

            // get segment and packet number
            segment_number = (packet[0] >> 4) & 0b00000111;
            packet_number = packet[1];

            fprintf(stderr, "%d got %d\n", pak, packet_number);

            if (pak != packet_number) { // out of sync
                pak = -1;
                mismatches++;
                if (mismatches == 100) {
                    mismatches = 0;
                    close(spi_fd);
                    usleep(200*1000);
                    open_spi_port(spi_path);
                }
                continue;
            }

            if (pak == 20) { // segment number is only valid on packet 20
                if (0 < segment_number && segment_number < 5) {
                    seg = segment_number; 
                }
                fprintf(stderr, "segment %d\n", seg);
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
    }
}

int main(int argc, char *argv[]) {
    // setup
    memset(image_ptr, 0, IMAGE_HEIGHT*IMAGE_WIDTH*sizeof(uint16_t));

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
    // for (int i = 0; i < 5; i++)
    read_image(image_ptr);

    // Print to stdout
    for (int i = 0; i < IMAGE_HEIGHT; i++) {
        for (int j = 0; j < IMAGE_WIDTH; j++) {
            printf("%d ", image[i][j]);
        }
        printf("\n");
    }

    return 0;
}


// read argv for spi number (should be 0 or 1)
static int set_spi_number(const char *arg) {
    int spi_number = atoi(arg);
    switch (spi_number) { // SPI bus number
        case 0:
            strcpy(spi_path, "/dev/spidev0.0");
            break;
        case 1:
            strcpy(spi_path, "/dev/spidev0.1");
            break;
        default:
            fprintf(stderr, "Invalid SPI number %d. Options are 0 or 1.\n", 
                            spi_number);
            return -1;
    }
    return 0;
}

// Open SPI port and do all the ioctl setup
static int open_spi_port(const char *path) {
    int ret;
	int spi_fd = open(path, O_RDWR);
	if (spi_fd < 0)
		pabort("can't open device");

	ret = ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(spi_fd, SPI_IOC_RD_MODE, &spi_mode);
	if (ret == -1)
		pabort("can't get spi mode");

	ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits_per_word);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bits_per_word);
	if (ret == -1)
		pabort("can't get bits per word");

	ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
	if (ret == -1)
		pabort("can't get max speed hz");
    return spi_fd;
}

// Exit program with error message
static void pabort(const char *s) {
	perror(s);
	abort();
}
