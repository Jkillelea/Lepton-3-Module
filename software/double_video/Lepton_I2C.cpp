#include <QtDebug>
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

bool _connected = false;

LEP_CAMERA_PORT_DESC_T _port;
LEP_SYS_FPA_TEMPERATURE_KELVIN_T fpa_temp_kelvin;
LEP_RESULT result;

int default_i2c_num = 1;

int lepton_connect(int i2c_num) {
	int res = (int) LEP_OpenPort(i2c_num, LEP_CCI_TWI, 400, &_port);
    if (res != LEP_OK) {
        qDebug() << "Failed to connect!";
        qDebug() << "error code: " << res << "";
    } else {
        _connected = true;
    }
	return res;
}

int lepton_enable_radiometry() {
    if (!_connected)
        lepton_connect(default_i2c_num);

    int res = LEP_SetRadEnableState(&_port, LEP_RAD_ENABLE);
    if (res != LEP_OK) {
        qDebug() << "Failed to enable radiometry";
        qDebug() << "error code: " << res << "";
    }
    return res;
}

int lepton_temperature(){
	if(!_connected)
		lepton_connect(default_i2c_num);
	result = ((LEP_GetSysFpaTemperatureKelvin(&_port, &fpa_temp_kelvin)));
	// printf("FPA temp kelvin: %i, code %i\n", fpa_temp_kelvin, result);
	return (fpa_temp_kelvin/100);
}


float raw2Celsius(float raw){
	float ambientTemperature = 25.0;
	float slope = 0.0217;
	return slope*raw+ambientTemperature-177.77;
}

void lepton_perform_ffc() {
	qDebug() << "performing FFC...";
	if(!_connected) {
		int res = lepton_connect(default_i2c_num);
		if (res != 0) {
			//check SDA and SCL lines if you get this error
			qDebug() << "I2C could not connect";
			qDebug() << "error code: " << res;
		}
	}

    int res = (int)LEP_RunSysFFCNormalization(&_port);
    if (res != 0) {
        qDebug() << "FFC not successful";
        qDebug() << "error code: " << res;
    } else {
        qDebug() << "FFC successful";
    }
}

void lepton_restart() {
	if(!_connected) {
		int res = lepton_connect(default_i2c_num);
		if (res != 0) {
			//check SDA and SCL lines if you get this error
			qDebug() << "I2C could not connect";
			qDebug() << "error code: " << res;
		}
	}

	qDebug() << "restarting...";
	Result res = LEP_RunOemReboot(&_port);

	if(res != LEP_OK)
		qDebug() << "restart unsuccessful with error: " << res;
	else
		qDebug() << "restart successful!";
}
