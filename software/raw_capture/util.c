// part of raw_capture. Copyright 2019 Jacob Killelea <jkillelea@protonmail.ch>
#include "util.h"
#include "global_vars.h"

// read argv for spi number (should be 0 or 1)
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

// Open SPI port and do all the ioctl setup
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

// Exit program with error message
void pabort(const char *s) {
    perror(s);
    abort();
}
