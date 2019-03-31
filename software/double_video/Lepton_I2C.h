#ifndef LEPTON_I2C
#define LEPTON_I2C

#include <stdio.h>
#include <stdbool.h>
#include "Lepton_I2C.h"
#include "lepton-sdk-fork/LEPTON_SDK.h"
#include "lepton-sdk-fork/LEPTON_ErrorCodes.h"
#include "lepton-sdk-fork/LEPTON_SYS.h"
#include "lepton-sdk-fork/LEPTON_Types.h"
#include "lepton-sdk-fork/LEPTON_AGC.h"
#include "lepton-sdk-fork/LEPTON_RAD.h"
#include "lepton-sdk-fork/LEPTON_OEM.h"

int lepton_connect(int i2c_num);
void lepton_perform_ffc();
int lepton_enable_radiometry();
int lepton_temperature();
float raw2Celsius(float);
void lepton_restart();

#endif
