#pragma once

#include <Arduino.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <SparkFun_SCD4x_Arduino_Library.h>
#include "Config.h"

class FRCManager
{
private:
    float buffer[STABILITY_BUFFER_LEN];
    int   count = 0;

    bool          doneTonight      = false;
    char          lastCalTimestamp[20] = "Never";
    String        lastCalDate      = "";
    unsigned long lastNearMissLog  = 0;

    Preferences prefs;

    float computeMean() const
    {
        float m = 0.0f;
        for (int i = 0; i < count; i++)
            m += buffer[i];
        return m / count;
    }

    float computeStdDev(float mean) const
    {
        float v = 0.0f;
        for (int i = 0; i < count; i++)
        {
            float d = buffer[i] - mean;
            v += d * d;
        }
        return sqrtf(v / count);
    }

    void logToSpiffs(const struct tm &timeinfo, float correction, float preCalMean)
    {
        File f = SPIFFS.open(FRC_LOG_PATH, FILE_APPEND);
        if (!f)
        {
            Serial.println("[SPIFFS] Failed to open FRC log for writing");
            return;
        }

        char tsBuf[25];
        strftime(tsBuf, sizeof(tsBuf), "%Y-%m-%dT%H:%M:%S", &timeinfo);

        f.printf("%s,%.1f,%.1f\n", tsBuf, correction, preCalMean);
        f.close();

        Serial.printf("[SPIFFS] Logged FRC event: %s, correction=%.1f, mean=%.1f\n",
                      tsBuf, correction, preCalMean);
    }

    bool attemptFRC(SCD4x &scd4x, const struct tm &timeinfo, float preCalMean)
    {
        Serial.println("[FRC] Attempting forced recalibration...");

        float correction = 0.0f;
        bool  ok         = scd4x.performForcedRecalibration(FRC_REFERENCE_PPM, &correction);

        if (!ok)
        {
            Serial.println("[FRC] Failed - sensor rejected calibration conditions.");
            return false;
        }

        Serial.printf("[FRC] Success. Correction applied: %.1f ppm\n", correction);

        if (fabsf(correction) > FRC_LARGE_CORRECTION_WARN_PPM)
            Serial.println("[FRC] WARNING: correction exceeds 150 ppm - "
                           "sensor may need inspection.");

        logToSpiffs(timeinfo, correction, preCalMean);

        strftime(lastCalTimestamp, sizeof(lastCalTimestamp), "%d %b %H:%M", &timeinfo);

        char todayStr[11];
        strftime(todayStr, sizeof(todayStr), "%Y-%m-%d", &timeinfo);
        lastCalDate = String(todayStr);

        prefs.putString("lastCalTs",   String(lastCalTimestamp));
        prefs.putString("lastCalDate", lastCalDate);

        doneTonight = true;
        return true;
    }

public:
    void begin()
    {
        prefs.begin("photocal", false);

        String ts = prefs.getString("lastCalTs", "Never");
        ts.toCharArray(lastCalTimestamp, sizeof(lastCalTimestamp));

        lastCalDate = prefs.getString("lastCalDate", "");
    }

    void pushReading(float filteredCo2)
    {
        if (count < STABILITY_BUFFER_LEN)
        {
            buffer[count++] = filteredCo2;
        }
        else
        {
            memmove(buffer, buffer + 1, (STABILITY_BUFFER_LEN - 1) * sizeof(float));
            buffer[STABILITY_BUFFER_LEN - 1] = filteredCo2;
        }
    }

    bool checkAndRun(SCD4x &scd4x, const struct tm &timeinfo)
    {
        bool inNightWindow = (timeinfo.tm_hour >= NIGHT_WINDOW_START_HOUR &&
                              timeinfo.tm_hour <  NIGHT_WINDOW_END_HOUR);

        if (!inNightWindow)
        {
            doneTonight = false;
            return false;
        }

        char todayStr[11];
        strftime(todayStr, sizeof(todayStr), "%Y-%m-%d", &timeinfo);
        if (lastCalDate == String(todayStr))
            doneTonight = true;

        if (doneTonight)               return false;
        if (count < STABILITY_BUFFER_LEN) return false;

        float mean   = computeMean();
        float stddev = computeStdDev(mean);

        bool stable = (stddev < STABILITY_STDDEV_THRESHOLD &&
                       mean  >= ATMOSPHERIC_MIN_PPM &&
                       mean  <= ATMOSPHERIC_MAX_PPM);

        if (!stable)
        {
            unsigned long now = millis();
            if (now - lastNearMissLog >= NEAR_MISS_LOG_INTERVAL_MS)
            {
                lastNearMissLog = now;
                Serial.printf("[FRC] Night window, conditions not met. "
                              "mean=%.1f ppm, stddev=%.1f ppm "
                              "(need %.0f-%.0f ppm, stddev<%.1f)\n",
                              mean, stddev, ATMOSPHERIC_MIN_PPM,
                              ATMOSPHERIC_MAX_PPM, STABILITY_STDDEV_THRESHOLD);
            }
            return false;
        }

        return attemptFRC(scd4x, timeinfo, mean);
    }

    void forceCalibration(SCD4x &scd4x, const struct tm &timeinfo)
    {
        Serial.println("[FRC] Force-triggered via Serial command "
                       "(bypassing night window / stability checks).");

        float preCalMean = (count > 0) ? computeMean() : 0.0f;
        attemptFRC(scd4x, timeinfo, preCalMean);
    }

    const char *getLastCalTimestamp() const { return lastCalTimestamp; }

    void dumpLog()
    {
        File f = SPIFFS.open(FRC_LOG_PATH, FILE_READ);
        if (!f)
        {
            Serial.println("[FRC] No calibration log found yet.");
            return;
        }

        Serial.println("[FRC] --- Calibration log (timestamp,correction_ppm,pre_cal_mean) ---");
        while (f.available())
            Serial.println(f.readStringUntil('\n'));
        Serial.println("[FRC] --- End of log ---");
        f.close();
    }
};