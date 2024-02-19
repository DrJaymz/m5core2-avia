#ifndef ESPNOW_VARS_H
#define ESPNOW_VARS_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "global.h"

bool sensorDataUpdated = false;
#define SENSOR_HISTORY_INTERVAL 5000 // 5 seconds
#define SENSOR_HISTORY_LENGTH 300

struct SensorData
{
    bool fuelQtyError;
    bool fuelPressError;
    bool oilPressError;
    bool oilTempError;
    float batteryVoltage;
    float fuelPress;
    float fuelLitres;
    float oilTemp;
    float oilPress;
    float amp;
    float cht1;
    bool ampError;
    int frame;
    unsigned long timestamp; // time in milliseconds
};

SensorData sensorData;

// Define the array to hold the history of readings
SensorData readings[SENSOR_HISTORY_LENGTH]; // 5 minutes x 6 readings per minute

// Define a variable to keep track of the current index in the array
int currentIndex = 0;

// Function to add a new reading to the array
void addReading(SensorData newReading)
{
    // Add the new reading to the array
    newReading.timestamp = millis();
    readings[currentIndex] = newReading;
    // Increment the currentIndex, wrapping around to 0 when it reaches the end of the array
    currentIndex = (currentIndex + 1) % 30;
}

class ESPNowReceiver
{
private:
    static void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int len)
    {
        static unsigned long nextSavedReadingTimestamp;

        if (len != sizeof(SensorData))
        {
            Serial.println("Invalid data received");
            return;
        }

        if (!SIMULATE)
        {
            memcpy(&sensorData, data, sizeof(sensorData));
        }

        if (millis() > nextSavedReadingTimestamp)
        {

            addReading(sensorData);
            nextSavedReadingTimestamp = millis() + SENSOR_HISTORY_INTERVAL;
            Serial.printf("i: rx %i ** saved %i **\n", sensorData.frame, currentIndex);
        }
        else
        {
            Serial.printf("i: rx %i\n", sensorData.frame);
        }

        // Serial.print("  MAC address: ");
        // for (int i = 0; i < 6; i++)
        // {
        //     Serial.print(mac_addr[i], HEX);
        //     if (i < 5)
        //     {
        //         Serial.print(":");
        //     }
        // }

        sensorDataUpdated = true;

    }

public:
    ESPNowReceiver()
    {
    }

    void debug()
    {
        Serial.println();
        Serial.print("  Fuel quantity error: ");
        Serial.println(sensorData.fuelQtyError ? "true" : "false");
        Serial.print("  Fuel pressure error: ");
        Serial.println(sensorData.fuelPressError ? "true" : "false");
        Serial.print("  Oil pressure error: ");
        Serial.println(sensorData.oilPressError ? "true" : "false");
        Serial.print("  Oil temperature error: ");
        Serial.println(sensorData.oilTempError ? "true" : "false");
        Serial.print("  Battery voltage: ");
        Serial.println(sensorData.batteryVoltage);
        Serial.print("  Fuel pressure: ");
        Serial.println(sensorData.fuelPress);
        Serial.print("  Fuel litres: ");
        Serial.println(sensorData.fuelLitres);
        Serial.print("  Oil temperature: ");
        Serial.println(sensorData.oilTemp);
        Serial.print("  Oil pressure: ");
        Serial.println(sensorData.oilPress);
    }

    bool init()
    {

        WiFi.mode(WIFI_STA);
        if (esp_now_init() != ESP_OK)
        {
            Serial.println("Error initializing ESP-NOW");
            return false;
        }
        else
        {
            Serial.println("ESP-NOW OK");
            esp_now_register_recv_cb(onDataReceived);
        }
        return true;
    }

    void pauseWiFi()
    {
        esp_now_deinit();
        WiFi.mode(WIFI_OFF);
        Serial.println("Wi-Fi paused");
    }

    bool resumeWiFi()
    {
        Serial.printf("Resume Wifi heap: %u\n",esp_get_free_heap_size());
        WiFi.mode(WIFI_STA);
        if (esp_now_init() == ESP_OK)
        {
            esp_now_register_recv_cb(onDataReceived);
            Serial.println("ESP-NOW resumed successfully");
            return true;
        }
        else
        {
            Serial.println("Error initializing ESP-NOW");
            return false;
        }
    }
};

#endif