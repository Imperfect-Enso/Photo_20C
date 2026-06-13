#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"

class WiFiTimeManager
{
private:
    bool          synced      = false;
    unsigned long lastAttempt = 0;

    bool attemptSync()
    {
        Serial.print("[WiFi] Connecting");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED &&
               (millis() - start) < WIFI_CONNECT_TIMEOUT_MS)
        {
            delay(250);
            Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("[WiFi] Connect failed - will retry later. "
                           "Device runs standalone in the meantime.");
            return false;
        }

        Serial.println("[WiFi] Connected. Syncing time via NTP...");
        configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, NTP_SERVER);

        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 10000))
        {
            Serial.println("[NTP] Time synced.");
            return true;
        }

        Serial.println("[NTP] Sync failed - will retry later.");
        return false;
    }

public:
    void begin()
    {
        lastAttempt = millis();
        synced      = attemptSync();
    }

    void update()
    {
        if (synced) return;

        unsigned long now = millis();
        if (now - lastAttempt >= WIFI_RETRY_INTERVAL_MS)
        {
            lastAttempt = now;
            Serial.println("[WiFi] Periodic retry...");
            synced = attemptSync();
        }
    }

    bool isSynced() const { return synced; }
};