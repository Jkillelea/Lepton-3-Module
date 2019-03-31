#include <QtDebug>
#include "LeptonI2CClass.h"

static int tries = 5;

LeptonI2CClass::LeptonI2CClass(int i2c_num) {
    qDebug() << "LeptonI2CClass(" << i2c_num << ")";
    this->_i2c_num = i2c_num;

    for (int i = 0; i < tries; i++) {
        if (connect() == LEP_OK)
            break;
    }
}

int LeptonI2CClass::connect() {
    qDebug() << "connect";
	LEP_RESULT res = LEP_OpenPort(this->_i2c_num, LEP_CCI_TWI, 400, this->_port);

    if (res != LEP_OK)
        qDebug() << "Failed to connect!\nerror code: " << error_as_string(res);
    else
        this->_connected = true;

    return res;
}

int LeptonI2CClass::enable_radiometry() {
    qDebug() << "enable_radiometry";
    if (!this->_connected)
        if(connect() != LEP_OK)
            return -1;

    LEP_RESULT res = LEP_SetRadEnableState(this->_port, LEP_RAD_ENABLE);

    if (res != LEP_OK) {
        qDebug() << "Failed to enable radiometry";
        qDebug() << "error code: " << error_as_string(res) << "";
    }
    return res;
}

int LeptonI2CClass::temperature() {
    qDebug() << "temperature";
    LEP_SYS_FPA_TEMPERATURE_KELVIN_T fpa_temp_kelvin;

	if(!this->_connected)
		connect();

	LEP_RESULT result = LEP_GetSysFpaTemperatureKelvin(this->_port, &fpa_temp_kelvin);

    if (result == LEP_OK)
        return (fpa_temp_kelvin/100);
    else
         return -1;
}

int LeptonI2CClass::perform_ffc() {
	qDebug() << "performing FFC...";

	if(!this->_connected) {
		if (connect() != LEP_OK)
			qDebug() << "I2C could not connect";
	}

    LEP_RESULT res = LEP_RunSysFFCNormalization(this->_port);

    if (res != LEP_OK)
        qDebug() << "FFC not successful\nerror code: " << error_as_string(res);
    else
        qDebug() << "FFC successful";

    return res;
}

int LeptonI2CClass::restart() {
    qDebug() << "restart";
	if(!this->_connected) {
		if (connect() != LEP_OK)
			qDebug() << "I2C could not connect";
	}

	qDebug() << "restarting...";
	LEP_RESULT res = LEP_RunOemReboot(this->_port);

	if(res != LEP_OK)
		qDebug() << "restart unsuccessful with error: " << error_as_string(res);
	else
		qDebug() << "restart successful!";

    return res;
}
