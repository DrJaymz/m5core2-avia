#ifndef TIMESTUFF_VARS_H
#define TIMESTUFF_VARS_H

#include <Arduino.h>
#include "global.h"

RTC_TimeTypeDef TimeStruct;
RTC_DateTypeDef DateStruct;
struct tm timeinfo;
const char *timezone = "GMT+0BST-1,M3.5.0/01:00:00,M10.5.0/02:00:00";
const char *ntpServer = "2.pool.ntp.org";

void getRtcTime(char *timeStr, size_t size)
{
    // Get the current date and time from the RTC
    RTC_DateTypeDef date;
    RTC_TimeTypeDef time;
    M5.Rtc.GetTime(&time);
    M5.Rtc.GetDate(&date);

    // Format the date and time as a string
    snprintf(timeStr, size, "%04d-%02d-%02d %02d:%02d:%02d",
             date.Year, date.Month, date.Date,
             time.Hours, time.Minutes, time.Seconds);
}

void getRtcFileName(char *timeStr, size_t size)
{
    // Get the current date and time from the RTC
    RTC_DateTypeDef date;
    RTC_TimeTypeDef time;
    M5.Rtc.GetTime(&time);
    M5.Rtc.GetDate(&date);

    // Format the date and time as a string
    snprintf(timeStr, size, "/%04d%02d%02d%02d%02d%02d.csv",
             date.Year, date.Month, date.Date,
             time.Hours, time.Minutes, time.Seconds);
}

bool timeSync()
{
    configTime(0, 0, ntpServer); // get UTC time from NTP server
    setenv("TZ", timezone, 1);   // Set the TZ.
    tzset();
    if (!getLocalTime(&timeinfo))
    {
        Serial.print("Failed to obtain time from ");
        Serial.println(ntpServer);
        return false;
    }
    // Copy the time and date from the struct tm format into the M5Core2
    // TimeStruct and write it into the RTC.
    TimeStruct.Hours = timeinfo.tm_hour;
    TimeStruct.Minutes = timeinfo.tm_min;
    TimeStruct.Seconds = timeinfo.tm_sec;
    DateStruct.Year = (timeinfo.tm_year + 1900);
    DateStruct.Month = (timeinfo.tm_mon + 1);
    DateStruct.Date = timeinfo.tm_mday;
    DateStruct.WeekDay = timeinfo.tm_wday; // day of week. 0 = Sunday
    M5.Rtc.SetTime(&TimeStruct);
    M5.Rtc.SetDate(&DateStruct);
    return true;
}

#endif