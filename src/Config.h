#pragma once

#include <Arduino.h>

#define SDA_PIN 21
#define SCL_PIN 22

static const char *WIFI_SSID                  = "TP-Link_C6";
static const char *WIFI_PASSWORD              = "mBt&t#7896#";
static const unsigned long WIFI_CONNECT_TIMEOUT_MS  = 10000UL;
static const unsigned long WIFI_RETRY_INTERVAL_MS   = 30UL * 60UL * 1000UL;

static const char *NTP_SERVER       = "pool.ntp.org";
static const long GMT_OFFSET_SEC    = 5 * 3600 + 1800;
static const int  DST_OFFSET_SEC   = 0;

static const unsigned long SENSOR_READ_INTERVAL_MS    = 5000UL;
static const unsigned long DISPLAY_REFRESH_INTERVAL_MS = 5000UL;

static const int           SPARKLINE_LEN          = 32;
static const unsigned long SPARKLINE_INTERVAL_MS  = 2UL * 60UL * 1000UL;

static const int AQI_GOOD_MAX         = 600;
static const int AQI_MODERATE_MAX     = 1000;
static const int AQI_HIGH_MAX         = 1500;
static const int VENTILATE_HYSTERESIS = 50;

static const uint16_t FRC_REFERENCE_PPM              = 420;
static const float    STABILITY_STDDEV_THRESHOLD      = 15.0f;
static const float    ATMOSPHERIC_MIN_PPM             = 390.0f;
static const float    ATMOSPHERIC_MAX_PPM             = 450.0f;
static const int      NIGHT_WINDOW_START_HOUR         = 0;
static const int      NIGHT_WINDOW_END_HOUR           = 4;
static const int      STABILITY_BUFFER_LEN            = 360;
static const float    FRC_LARGE_CORRECTION_WARN_PPM   = 150.0f;
static const unsigned long NEAR_MISS_LOG_INTERVAL_MS  = 5UL * 60UL * 1000UL;

static const char *FRC_LOG_PATH = "/frc_log.csv";

static const int MAX_SENSOR_FAILURES = 5;

static const uint8_t OLED_I2C_ADDR   = 0x3C;
static const uint8_t BME280_I2C_ADDR = 0x76;