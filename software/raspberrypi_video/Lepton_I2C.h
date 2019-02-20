#ifndef LEPTON_I2C
#define LEPTON_I2C

int lepton_connect(int i2c_num);
void lepton_perform_ffc();
int lepton_enable_radiometry();
int lepton_temperature();
float raw2Celsius(float);
void lepton_restart();

#endif
