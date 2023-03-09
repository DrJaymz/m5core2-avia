#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

#include <Arduino.h>
#include <TelnetSpy.h>
#include "espnow.h"
//#include <TFT_eSPI.h>

//intellisense workaround 
// _VOID      _EXFUN(tzset,	(_VOID));
// int	_EXFUN(setenv,(const char *__string, const char *__value, int __overwrite));

#define HOSTNAME "AVIA"

extern TelnetSpy debug;
extern SensorData sensorData;

// Font files are stored in SPIFFS, so load the library
#include <FS.h>

#endif