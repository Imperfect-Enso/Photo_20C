#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "AQIClassifier.h"
#include "Config.h"

using OledDriver = U8G2_SH1106_128X64_NONAME_F_HW_I2C;

class DisplayManager
{
private:
    OledDriver u8g2;

    static constexpr const uint8_t *FONT_BIG   = u8g2_font_logisoso16_tn;
    static constexpr const uint8_t *FONT_MED   = u8g2_font_6x10_tf;
    static constexpr const uint8_t *FONT_SMALL = u8g2_font_5x7_tf;

    static constexpr int Y_CO2_BASE  = 18;
    static constexpr int Y_PPM_BASE  = 26;
    static constexpr int Y_BAND_BASE = 26;
    static constexpr int Y_DIV1      = 22;
    static constexpr int Y_SPARK_TOP = 24;
    static constexpr int Y_SPARK_BOT = 47;
    static constexpr int Y_DIV2      = 49;
    static constexpr int Y_ENV_BASE  = 57;
    static constexpr int Y_CAL_BASE  = 63;

    void hline(int y)
    {
        u8g2.drawHLine(0, y, 128);
    }

    void drawSparkline(const float *buf, int len)
    {
        if (len < 2) return;

        float minV = buf[0];
        float maxV = buf[0];

        for (int i = 1; i < len; i++)
        {
            if (buf[i] < minV) minV = buf[i];
            if (buf[i] > maxV) maxV = buf[i];
        }

        if (maxV - minV < 20.0f)
        {
            float mid = (maxV + minV) / 2.0f;
            minV = mid - 10.0f;
            maxV = mid + 10.0f;
        }

        const int   h     = Y_SPARK_BOT - Y_SPARK_TOP;
        const float xStep = 127.0f / (len - 1);

        for (int i = 0; i < len - 1; i++)
        {
            int x1 = (int)(i * xStep);
            int x2 = (int)((i + 1) * xStep);

            int y1 = Y_SPARK_BOT - (int)((buf[i]     - minV) / (maxV - minV) * h);
            int y2 = Y_SPARK_BOT - (int)((buf[i + 1] - minV) / (maxV - minV) * h);

            y1 = constrain(y1, Y_SPARK_TOP, Y_SPARK_BOT);
            y2 = constrain(y2, Y_SPARK_TOP, Y_SPARK_BOT);

            u8g2.drawLine(x1, y1, x2, y2);
        }
    }

public:
    DisplayManager() : u8g2(U8G2_R0, U8X8_PIN_NONE) {}

    void begin()
    {
        u8g2.begin();
        u8g2.setContrast(220);
    }

    void showSplash(const char *line1, const char *line2)
    {
        u8g2.clearBuffer();
        u8g2.setFont(FONT_MED);
        u8g2.drawStr((128 - u8g2.getStrWidth(line1)) / 2, 28, line1);
        u8g2.drawStr((128 - u8g2.getStrWidth(line2)) / 2, 44, line2);
        u8g2.sendBuffer();
    }

    void showSensorError()
    {
        u8g2.clearBuffer();
        u8g2.setFont(FONT_MED);

        const char *e1 = "SENSOR ERROR";
        const char *e2 = "Check wiring";
        const char *e3 = "Retrying...";

        u8g2.drawStr((128 - u8g2.getStrWidth(e1)) / 2, 20, e1);
        hline(23);
        u8g2.drawStr((128 - u8g2.getStrWidth(e2)) / 2, 38, e2);
        u8g2.setFont(FONT_SMALL);
        u8g2.drawStr((128 - u8g2.getStrWidth(e3)) / 2, 52, e3);
        u8g2.sendBuffer();
    }

    void drawMain(float co2, float t, float h, float p, AQIBand band,
                  bool ventilate, const float *sparkBuf, int sparkLen,
                  const char *lastCal)
    {
        (void)ventilate;

        u8g2.clearBuffer();

        char co2Str[8];
        snprintf(co2Str, sizeof(co2Str), "%d", (int)co2);

        u8g2.setFont(FONT_BIG);
        u8g2.drawStr(0, 18, co2Str);

        int co2Width = u8g2.getStrWidth(co2Str);

        u8g2.setFont(FONT_SMALL);
        u8g2.drawStr(co2Width + 4, 18, "ppm");

        u8g2.setFont(FONT_MED);
        const char *label      = AQIClassifier::bandLabel(band);
        int         labelWidth = u8g2.getStrWidth(label);
        u8g2.drawStr(127 - labelWidth, 18, label);

        hline(Y_DIV1);
        drawSparkline(sparkBuf, sparkLen);
        hline(Y_DIV2);

        u8g2.setFont(FONT_SMALL);

        char envStr[32];
        snprintf(envStr, sizeof(envStr), "%.1fC  %d%%  %dhPa", t, (int)h, (int)p);
        u8g2.drawStr(0, Y_ENV_BASE, envStr);

        char calStr[24];
        snprintf(calStr, sizeof(calStr), "Cal:%s", lastCal);
        u8g2.drawStr(0, Y_CAL_BASE, calStr);

        u8g2.sendBuffer();
    }
};