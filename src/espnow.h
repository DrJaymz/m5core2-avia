#ifndef ESPNOW_VARS_H
#define ESPNOW_VARS_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>


bool sensorDataUpdated = false;

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
    int frame;
};

SensorData sensorData;

class ESPNowReceiver
{
private:
    static void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int len)
    {
        if (len != sizeof(SensorData))
        {
            Serial.println("Invalid data received");
            return;
        }
        
        memcpy(&sensorData, data, sizeof(sensorData));

        Serial.printf("i: rx %i\n", sensorData.frame);
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
};

#endif