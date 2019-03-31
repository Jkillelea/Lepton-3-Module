#ifndef LEPTONI2CCLASS_H_
#define LEPTONI2CCLASS_H_

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

class LeptonI2CClass {
    public:
        LeptonI2CClass(int i2c_num = 1);
        int connect();
        int perform_ffc();
        int enable_radiometry();
        int temperature();
        int restart();

    private:
        LEP_CAMERA_PORT_DESC_T_PTR _port;
        bool                       _connected;
        int                        _i2c_num;
};

#endif // LEPTONI2CCLASS_H_
