#include <Arduino.h>
#include <Wire.h>
#include <SPIFFS.h>
#include <SparkFun_SCD4x_Arduino_Library.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

#include "Config.h"
#include "KalmanFilter.h"
#include "AQIClassifier.h"
#include "FRCManager.h"
#include "WiFiTimeManager.h"
#include "DisplayManager.h"
#include "SerialCommands.h"

SCD4x scd4x;
Adafruit_BME280 bme;

DisplayManager display;
AQIClassifier aqi;
FRCManager frc;
WiFiTimeManager wifiTime;

KalmanFilter co2F(0.5f, 900.0f, 420.0f, 1000.0f);
KalmanFilter tF(0.01f, 0.5f);
KalmanFilter hF(0.05f, 1.0f);
KalmanFilter pF(0.1f, 2.0f);

unsigned long lastSensorRead = 0;
unsigned long lastDisplayRefresh = 0;
unsigned long lastSparklineSample = 0;

int consecutiveSensorFailures = 0;

float lastCo2 = 420.0f;
float lastTemp = 0.0f;
float lastHumidity = 0.0f;
float lastPressure = 1013.25f;

float sparklineBuffer[SPARKLINE_LEN];
int sparklineCount = 0;

static float compensateCO2(float rawCo2, float pressureHpa)
{
    const float P_REF = 1013.25f;

    if (pressureHpa < 800.0f || pressureHpa > 1100.0f)
    {
        return rawCo2;
    }

    return rawCo2 * (P_REF / pressureHpa);
}

static void pushSparklineSample(float value)
{
    if (sparklineCount < SPARKLINE_LEN)
    {
        sparklineBuffer[sparklineCount++] = value;
    }
    else
    {
        memmove(
            sparklineBuffer,
            sparklineBuffer + 1,
            (SPARKLINE_LEN - 1) * sizeof(float)
        );
        sparklineBuffer[SPARKLINE_LEN - 1] = value;
    }
}

void setup()
{
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);

    display.begin();
    display.showSplash("ANTAREZ", "Starting...");

    if (!bme.begin(BME280_I2C_ADDR) || !scd4x.begin())
    {
        display.showSensorError();
        while (1)
        {
            delay(100);
        }
    }

    scd4x.startPeriodicMeasurement();

    if (!SPIFFS.begin(true))
    {
        Serial.println("[SPIFFS] Mount failed - calibration log disabled");
    }

    frc.begin();
    wifiTime.begin();

    display.showSplash("ANTAREZ", "READY");
    delay(1000);

    Serial.println("[BOOT] Ready. Type 'help' for the Serial command list.");
}

void loop()
{
    unsigned long now = millis();

    if (now - lastSensorRead >= SENSOR_READ_INTERVAL_MS)
    {
        lastSensorRead = now;

        float rawTemp = bme.readTemperature();
        float rawHum = bme.readHumidity();
        float rawPres = bme.readPressure() / 100.0F;

        if (!isnan(rawTemp))
        {
            lastTemp = tF.update(rawTemp);
        }

        if (!isnan(rawHum))
        {
            lastHumidity = hF.update(rawHum);
        }

        if (!isnan(rawPres))
        {
            lastPressure = pF.update(rawPres);
        }

        if (scd4x.readMeasurement())
        {
            consecutiveSensorFailures = 0;

            float co2Raw = scd4x.getCO2();
            float co2Compensated = compensateCO2(co2Raw, lastPressure);
            lastCo2 = co2F.update(co2Compensated);

            frc.pushReading(lastCo2);
        }
        else
        {
            consecutiveSensorFailures++;
            Serial.println("[SCD40] Read failed - keeping last estimate");
        }
    }

    if (now - lastSparklineSample >= SPARKLINE_INTERVAL_MS)
    {
        lastSparklineSample = now;
        pushSparklineSample(lastCo2);
    }

    if (now - lastDisplayRefresh >= DISPLAY_REFRESH_INTERVAL_MS)
    {
        lastDisplayRefresh = now;

        if (consecutiveSensorFailures >= MAX_SENSOR_FAILURES)
        {
            display.showSensorError();
        }
        else
        {
            int co2Int = static_cast<int>(lastCo2);
            AQIBand band = AQIClassifier::classify(co2Int);
            bool ventilate = aqi.updateVentilateAlert(co2Int);

            int sparkLen = sparklineCount;
            if (sparkLen < 2)
            {
                pushSparklineSample(lastCo2);
                sparkLen = sparklineCount;
            }

            display.drawMain(
                lastCo2,
                lastTemp,
                lastHumidity,
                lastPressure,
                band,
                ventilate,
                sparklineBuffer,
                sparkLen,
                frc.getLastCalTimestamp()
            );
        }
    }

    wifiTime.update();

    if (wifiTime.isSynced())
    {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            frc.checkAndRun(scd4x, timeinfo);
        }
    }

    handleSerialCommands(frc, scd4x, co2F, wifiTime);
}