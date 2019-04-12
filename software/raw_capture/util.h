// part of raw_capture. Copyright 2019 Jacob Killelea <jkillelea@protonmail.ch>
#ifndef _RAW_CAPTURE_UTIL_H_
#define _RAW_CAPTURE_UTIL_H_

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


// Functions
void pabort(const char *s);
int open_spi_port(const char *path);
int set_spi_number(const char *arg);

#endif // _RAW_CAPTURE_UTIL_H_
