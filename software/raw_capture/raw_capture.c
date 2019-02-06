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
static size_t    max_offset = sizeof(image) / sizeof(uint16_t);

// I2C vars
static uint16_t i2c_number;
static LEP_CAMERA_PORT_DESC_T i2c_port;

// SPI protocol vars
static int     spi_fd    = -1;
static int     spi_speed = 10000000;
static uint8_t spi_mode  = SPI_MODE_3;
static char    spi_path[255];
static uint8_t spi_bits_per_word = 8;

// SPI communication vars
static uint8_t packet[PACKET_SIZE] = {0xff};
static int resets = 0;

// Functions
static void pabort(const char *s);
static int open_spi_port(const char *path);
static int set_spi_number(const char *arg);
// static void lepton_reboot_and_configure();

int main(int argc, char *argv[]) {
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

    if (LEP_SetSysTelemetryEnableState(&i2c_port, LEP_TELEMETRY_DISABLED) != LEP_OK)
        pabort("Couldn't disable telemetry!");

    if (LEP_SetRadEnableState(&i2c_port, LEP_RAD_ENABLE) != LEP_OK)
        pabort("Couldn't enable radiometry!");


    // SPI bus number
    if(set_spi_number(argv[2]) < 0)
        return -1;

    spi_fd = open_spi_port(spi_path);

    for (uint32_t seg = 1; seg <= NUM_SEGMENTS; seg++) {
        for (uint32_t pak = 0; pak < PACKETS_PER_SEGMENT; pak++) {

            if (read(spi_fd, packet, PACKET_SIZE) != PACKET_SIZE)
                fprintf(stderr, "SPI failed to read enough bytes!\n");

            // // Handle drop packets
            // if ((packet[0] & 0x0f) == 0x0f) {
            //     fprintf(stderr, "drop %x\n", packet[0]);
            //     pak--;
            //     continue;
            // }

            uint8_t segment_number = seg;
            // uint8_t segment_number = (packet[0] >> 4) & 0b00000111;
            // uint16_t packet_number = (packet[0] << 4) | packet[1];
            uint16_t packet_number  = packet[1];

            if (packet_number != pak) {
                pak--;
                resets++;

                if (resets > 100) {
                    resets = 0;
                    seg = 1;
                    pak = 0;
                    close(spi_fd);
                    usleep(200000);
                    open_spi_port(spi_path);
                }
                continue;
            }

            fprintf(stderr, "%d %d\n", segment_number, packet_number);

            size_t offset = 80*packet_number + 4800*(segment_number-1);

            // fprintf(stderr, "%d %d\n", offset, max_offset);

            // if (offset > max_offset)
            //     continue;

            for (int i = 0; i < 80; i++) {
                size_t idx = 2*i + 4;
                image_ptr[offset + i] = packet[idx] << 8 | packet[idx + 1];
            }
        }
    }

    for (int i = 0; i < IMAGE_HEIGHT; i++) {
        for (int j = 0; j < IMAGE_WIDTH; j++) {
            printf("%d ", image[i][j]);
        }
        printf("\n");
    }

    return 0;
}


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


static void pabort(const char *s) {
	perror(s);
	abort();
}
