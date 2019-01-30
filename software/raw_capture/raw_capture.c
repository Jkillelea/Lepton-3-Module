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
#include <LEPTON_ErrorCodes.h>

#define IMAGE_WIDTH (160)
#define IMAGE_HEIGHT (120)
#define FRAME_SIZE (2*IMAGE_WIDTH + 4)

uint16_t image[IMAGE_HEIGHT][IMAGE_WIDTH];

int i2c_number;

int     spi_fd = -1;
char    spi_path[255];
int     spi_speed = 20000000;
uint8_t spi_mode = SPI_MODE_3;
uint8_t spi_bits_per_word = 8;

static void pabort(const char *s) {
	perror(s);
	abort();
}

// int set_i2c_number(const char *arg) {
//     int i2c_number = atoi(arg);
//     switch (i2c_number) { // I2C bus number
//         case 0:
//             strcpy(i2c_path, "/dev/i2c-0");
//             break;
//         case 1:
//             strcpy(i2c_path, "/dev/i2c-1");
//             break;
//         default:
//             fprintf(stderr, "Invalid I2C number %d. Options are 0 or 1.\n", 
//                             i2c_number);
//             return -1;
//     }
//     return 0;
// }

int set_spi_number(const char *arg) {
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

int open_spi_port(const char *path) {
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

int spi_transfer(int fd) {
	int ret;
	int i;
	uint8_t frame_number = 0;
	uint16_t delay = 0;
	uint8_t tx[FRAME_SIZE] = {0, };
	uint8_t lepton_frame_packet[FRAME_SIZE] = {0, };

	// struct spi_ioc_transfer tr = {
	// 	.tx_buf = (unsigned long)tx,
	// 	.rx_buf = (unsigned long)lepton_frame_packet,
	// 	.len = FRAME_SIZE,
	// 	.delay_usecs = delay,
	// 	.speed_hz = spi_speed,
	// 	.bits_per_word = spi_bits_per_word,
	// };

	// ret = ioctl(fd, SPI_IOC_MESSAGE(4), &tr);
    ret = read(fd, lepton_frame_packet, sizeof(lepton_frame_packet));
	if (ret < 1)
		pabort("can't send spi message");

	if(((lepton_frame_packet[0] & 0xf) != 0x0f)) {
		frame_number = lepton_frame_packet[1];
		if(frame_number < IMAGE_HEIGHT) {
			for(i = 0; i < IMAGE_WIDTH; i++) {
				image[frame_number][i] = 
                        (lepton_frame_packet[2*i+4] << 8 
                       | lepton_frame_packet[2*i+5]);
			}
		}
	}
    usleep(1000);
	return frame_number;
}


int main(int argc, char *argv[]) {
    // parse opts
    if (argc < 3) {
        fprintf(stderr, "Usage: %s i2c-number spi-number\n", argv[0]);
        return -1;
    }

    if(set_spi_number(argv[2]) < 0)
        return -1;

    fprintf(stderr, "SPI Path %s\n", spi_path);

    spi_fd = open_spi_port(spi_path);

    // make the transfer
    while(spi_transfer(spi_fd) != 59);


    for (int i = 0; i < IMAGE_HEIGHT; i++) {
        for (int j = 0; j < IMAGE_WIDTH; j++) {
            printf("%d ", image[i][j]);
        }
        printf("\n");
    }

    return 0;
}
