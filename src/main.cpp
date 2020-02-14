#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <uRTCLib.h>
#include <NTPClient.h>
#include "display.h"

#define NTP_SERVER "ch.pool.ntp.org"// NTP server to use
#define WIFI_TIMEOUT      5000      // Number of [ms] to wait for WiFi connection
#define START_TIME  1527458400      // from which date to count up [timestamp] - 2018_05_27
#define DAYS_DURATION     7500      // How long to show the number of days [ms]
#define uS_TO_S_FACTOR 1000000      // Conversion factor for micro seconds to seconds


uRTCLib rtc;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER);


// millis() from when we started to show days-since-screen
// used to make sure we always hide it around the same time
unsigned long start;


/**
 * rtc_to_sys()
 * Reads date/time from RTC module and sets internal system time accordingly.
 */
void rtc_to_sys() {
    rtc.refresh();

    // Build tm struct so we can use mktime to get an epoch value
    tm rtc_time = {
        rtc.second(), rtc.minute(),  rtc.hour(),
        rtc.day(),    rtc.month()-1, rtc.year()+100
    };
    timeval time = {mktime(&rtc_time), 0};
    settimeofday(&time, NULL);
}


/**
 * setup()
 * Initialize RTC clock and read time to internal system clock.
 * Switch display to days_since and start WiFi connection.
 */
void setup() {
    Serial.begin(115200);

    // First get RTC time as this one is always available (and fast)
    Wire.begin(25, 26);
    rtc_to_sys();
    tm local;
    getLocalTime(&local);
    Serial.println(&local, "RTC: %d.%m.%Y %H:%M:%S");

    // Switch to 'days ago' mode on display and record when we started
    start = millis();
    show_display(DD_DAYS_SINCE, (time(NULL) - START_TIME) / 86400);

    // Start WiFi
    Serial.print("Connecting to WiFi... ");
    WiFi.begin(); // Use stored cretentials from somewhere in the past

    // Wait for connection or timeout
    unsigned long wifi_start = millis();
    while ( WiFi.status() != WL_CONNECTED && millis() - wifi_start < WIFI_TIMEOUT) {
        delay (10);
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("connected");
    } else {
        Serial.println("FAILED!");
    }
}


void loop() {
    timeClient.update();

    static bool time_received = false;
    if (time_received == false && timeClient.getEpochTime() > 1000) { // 1000 chosen randomly - seems to work
        time_received = true;
        Serial.printf("Epoch looks good: %lu\n", timeClient.getEpochTime());
        
        // Setting system time as NTPClient does not do that
        Serial.println("Setting system time and RTC module");
        timeval time = {(time_t)timeClient.getEpochTime(), 0};
        settimeofday(&time, NULL);

        // Now fill tm struct with the date using system clock and update rtc module
        struct tm local;
        getLocalTime(&local);
        rtc.set(local.tm_sec, local.tm_min, local.tm_hour, local.tm_wday, local.tm_mday, local.tm_mon+1, local.tm_year - 100);
    }

    // Skip the rest - only doing it DAYS_DURATION after we showed the days_since screen
    if (millis() - start <= DAYS_DURATION)
        return;
    
    // Reset display to boot mode
    show_display(DD_BOOT);

    // Calculate number of seconts till midnight
    time_t sleep_s = 86400 - time(NULL) % 86400;
    Serial.print("Going to sleep for [s]: ");
    Serial.println(sleep_s);

    // Shuting everything down and go into deep sleep
    Serial.flush(); 
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    esp_sleep_enable_timer_wakeup((uint64_t)sleep_s * uS_TO_S_FACTOR);
    esp_deep_sleep_start();

    // NOTE: Deep Sleep still uses about 7.6mA on this specific dev board!
}