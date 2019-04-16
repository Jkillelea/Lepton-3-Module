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

void pabort(const char *msg) {
    perror(msg);
    abort();
}

int main(int argc, char *argv[]) {
    // parse opts
    if (argc < 2) {
        fprintf(stderr, "Usage: %s i2c-number\n", argv[0]);
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

    if (LEP_RunOemReboot(&i2c_port) != LEP_OK)
        pabort("Couldn't reboot!");

    // LEP_STATUS_T status;
    // LEP_GetSysStatus(&i2c_port, &status);


    // Disable telemetry (changes packet lengths)
    if (LEP_SetSysTelemetryEnableState(&i2c_port, LEP_TELEMETRY_DISABLED) != LEP_OK)
        pabort("Couldn't disable telemetry!");

    // Enable radiometry
    if (LEP_SetRadEnableState(&i2c_port, LEP_RAD_ENABLE) != LEP_OK)
        pabort("Couldn't enable radiometry!");

}


