#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
#include <M5Core2.h>
#include "global.h"
#include <SD.h>

char fileName[20];
extern char timeStr[20];
extern float accX, accY, accZ;
bool sdPresent = false;

bool beginSD()
{
    int retry = 0;

    bool retVal = SD.begin(TFCARD_CS_PIN, SPI, 40000000);
    while (retry < 5 && !SD.begin(TFCARD_CS_PIN, SPI, 40000000))
    {
        Serial.printf("info: SD card present: %d\n", retVal);
        retry++;
        delay(200);
    }
    Serial.printf("i: SD card present: %d\n", retVal);
    sdPresent = retVal;

    if (sdPresent)
    {
        getRtcFileName(fileName, 20);
        Serial.print("i: SD filename: ");
        Serial.println(fileName);

        File dataFile = SD.open(fileName, FILE_WRITE);
        getRtcTime(timeStr, sizeof(timeStr));
        dataFile.printf("timeStr,frame,batteryVoltage,amp,fuelLitres,fuelPress,oilTemp,oilPress,cht1,accX,accY,accZ\n");
        dataFile.close();
    }

    return retVal;
}

bool checkSD()
{
    if (sdPresent)
    {
        bool retVal = SD.exists(fileName);
        // once not present forever not present.
        sdPresent = retVal;
        return retVal;
    }
    else
        return false;
}

bool writeSD()
{
    if (checkSD())
    {
        File dataFile = SD.open(fileName, FILE_APPEND);

        dataFile.printf("%s,%i,%0.2f,%0.2f,%0.1f,%0.1f,%0.1f,%0.1f,%0.1f,%0.1f,%0.1f,%0.1f\n",
                        timeStr,
                        sensorData.frame,
                        sensorData.batteryVoltage,
                        sensorData.amp,
                        sensorData.fuelLitres,
                        sensorData.fuelPress,
                        sensorData.oilTemp,
                        sensorData.oilPress,
                        sensorData.cht1,
                        accX, accY, accZ);

        dataFile.close();
        return true;
    }
    return false;
}

#endif