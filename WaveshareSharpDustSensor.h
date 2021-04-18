// WaveshareSharpDustSensor.h

#ifndef _WAVESHARESHARPDUSTSENSOR_h
#define _WAVESHARESHARPDUSTSENSOR_h

#define        COV_RATIO                       0.2            //ug/mmm / mv
#define        NO_DUST_VOLTAGE                 400            //mv
#define        SYS_VOLTAGE                     3300           
class WaveshareSharpDustSensor{
public:
	WaveshareSharpDustSensor::WaveshareSharpDustSensor();
	float density;
	float voltage;
	int Filter(int m);
	/// Convert raw ADC voltage reading (0-1023) into dust density in ug/m^3
	float Conversion(int rawVoltageADC);
	
};




#endif