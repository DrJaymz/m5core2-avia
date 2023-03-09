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

struct ColoredRange {
  float start;
  float end;
  uint32_t color;
};

ColoredRange oilTempRange[] = {
  {0, 40, RED},
  {120, 140, RED},
};
int oilTempRangeNum = 2;


ColoredRange oilPressRange[] = {
  {0, 1.5, RED},
  {1.5, 4.5, YELLOW},
  {4.5, 6.2, GREEN},
  {6.2, 7.0, RED},
};
int oilPressRangeNum = 4;

ColoredRange fuelPressRange[] = {
  {23, 26, RED},
  {26, 350, GREEN},
};
int fuelPressRangeNum = 2;

ColoredRange fuelQTYRange[] = {
  {0, 10, RED},
  {10, 20, YELLOW},
  {20, 120, GREEN},
};
int fuelQTYRangeNum = 3;



// Font files are stored in SPIFFS, so load the library
#include <FS.h>

#endif