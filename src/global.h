#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

#define HOSTNAME "AVIA"
#define SIMULATE 0
#define MUTE 1

#include <Arduino.h>
#include <TelnetSpy.h>
#include <M5Core2.h>
#include "espnow.h"
#include "timestuff.h"
#include "Core2_Sounds.h"
#include "sdcard.h"

//intellisense workaround 
// _VOID      _EXFUN(tzset,	(_VOID));
// int	_EXFUN(setenv,(const char *__string, const char *__value, int __overwrite));

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
  {0, 35, RED},
  {35, 350, DARKGREY},
};
int fuelPressRangeNum = 2;

ColoredRange fuelQTYRange[] = {
  {0, 10, RED},
  {10, 20, YELLOW},
  {20, 120, GREEN},
};
int fuelQTYRangeNum = 3;

int fuelQTYWarning = 0;
int fuelPressWarning = 0;
int oilPressWarning = 0;
int oilTempWarning = 0;
int alarmSound = 0;

float accX = 0.0F;  // Define variables for storing inertial sensor data
float accY = 0.0F;  
float accZ = 0.0F;

// Font files are stored in SPIFFS, so load the library
#include <FS.h>

#endif