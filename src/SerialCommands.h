#pragma once
/*
 * =====================================================================
 *  SerialCommands.h - line-based Serial command interface
 * =====================================================================
 *  Type a command into the Serial Monitor (115200 baud, newline
 *  ending) and press Enter:
 *
 *    help          - list commands
 *    status        - print Wi-Fi/NTP, Kalman Q/R, last calibration
 *    dump          - print the SPIFFS FRC calibration log
 *    force         - force an FRC attempt right now, bypassing the
 *                    night-window/stability/baseline gates (useful
 *                    for bench-testing the FRC call path without
 *                    waiting for 00:00-04:00 and a stable 30-min
 *                    window)
 *    setq <value>  - set the CO2 Kalman filter's process noise Q
 *    setr <value>  - set the CO2 Kalman filter's measurement noise R
 *
 *  This is non-blocking: handle() only does work if a full line is
 *  available, and is cheap to call every loop() iteration.
 * =====================================================================
 */

#include <Arduino.h>
#include <SparkFun_SCD4x_Arduino_Library.h>
#include "FRCManager.h"
#include "KalmanFilter.h"
#include "WiFiTimeManager.h"

inline void printSerialHelp()
{
    Serial.println("[CMD] Available commands:");
    Serial.println("  help          - show this message");
    Serial.println("  status        - show system status");
    Serial.println("  dump          - print FRC calibration log");
    Serial.println("  force         - force FRC now (bypass gating)");
    Serial.println("  setq <value>  - set CO2 Kalman process noise Q");
    Serial.println("  setr <value>  - set CO2 Kalman measurement noise R");
}

inline void handleSerialCommands(FRCManager &frc, SCD4x &scd4x,
                                  KalmanFilter &co2F, WiFiTimeManager &wifiTime)
{
    if (!Serial.available())
        return;

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.length() == 0)
        return;

    if (cmd == "help")
    {
        printSerialHelp();
    }
    else if (cmd == "status")
    {
        Serial.printf("[STATUS] WiFi/NTP synced : %s\n", wifiTime.isSynced() ? "yes" : "no");
        Serial.printf("[STATUS] Kalman Q=%.3f R=%.3f\n", co2F.getQ(), co2F.getR());
        Serial.printf("[STATUS] Last calibration: %s\n", frc.getLastCalTimestamp());
    }
    else if (cmd == "dump")
    {
        frc.dumpLog();
    }
    else if (cmd == "force")
    {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            frc.forceCalibration(scd4x, timeinfo);
        }
        else
        {
            Serial.println("[CMD] No valid time available (Wi-Fi/NTP not synced) - "
                            "cannot timestamp a forced FRC.");
        }
    }
    else if (cmd.startsWith("setq "))
    {
        float v = cmd.substring(5).toFloat();
        co2F.setQ(v);
        Serial.printf("[CMD] Kalman Q set to %.3f\n", v);
    }
    else if (cmd.startsWith("setr "))
    {
        float v = cmd.substring(5).toFloat();
        co2F.setR(v);
        Serial.printf("[CMD] Kalman R set to %.3f\n", v);
    }
    else
    {
        Serial.println("[CMD] Unknown command. Type 'help' for a list.");
    }
}
