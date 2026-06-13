#pragma once

#include <Arduino.h>
#include "Config.h"

enum AQIBand
{
    AQI_GOOD,
    AQI_MODERATE,
    AQI_HIGH,
    AQI_CRITICAL
};

class AQIClassifier
{
private:
    bool ventilateActive = false;

public:
    static AQIBand classify(int ppm)
    {
        if (ppm < AQI_GOOD_MAX)     return AQI_GOOD;
        if (ppm < AQI_MODERATE_MAX) return AQI_MODERATE;
        if (ppm < AQI_HIGH_MAX)     return AQI_HIGH;
        return AQI_CRITICAL;
    }

    static const char *bandLabel(AQIBand band)
    {
        switch (band)
        {
            case AQI_GOOD:     return "GOOD";
            case AQI_MODERATE: return "MODERATE";
            case AQI_HIGH:     return "HIGH";
            case AQI_CRITICAL: return "CRITICAL";
        }
        return "?";
    }

    bool updateVentilateAlert(int ppm)
    {
        if (!ventilateActive && ppm > AQI_MODERATE_MAX)
            ventilateActive = true;
        else if (ventilateActive && ppm < (AQI_MODERATE_MAX - VENTILATE_HYSTERESIS))
            ventilateActive = false;

        return ventilateActive;
    }
};