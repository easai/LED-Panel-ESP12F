#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>
#include "config.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 12

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

char ssid[] = SECRET_SSID;
char password[] = SECRET_PASS;
char url[] = SITE_URL;

const int BUZZ = 4; // buzzer pin
const int SW = 5;   // switch pin

volatile bool beepEnabled = true;
volatile bool toggleRequest = false;

unsigned long lastCheck = 0;
bool lastUp = true;
bool wifiMessageDone = false;

bool checkSite()
{
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();

    http.setTimeout(5000);
    if (!http.begin(client, url))
        return false;

    int code = http.GET();
    http.end();

    if (code <= 0)
        return false;

    return (code < 500);
}

void showMessage(const char *msg)
{
    P.displayClear();
    P.displayText(msg, PA_CENTER, 40, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    P.displayReset();
}

void IRAM_ATTR handleSwitch()
{
    toggleRequest = true;
}

void setup()
{
    pinMode(BUZZ, OUTPUT);
    pinMode(SW, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(SW), handleSwitch, FALLING);

    P.begin();
    P.setIntensity(2);
    P.displayClear();
    delay(300);

    // WiFi connect message ONCE
    showMessage("Wifi...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000)
        delay(200);

    if (WiFi.status() != WL_CONNECTED)
    {
        showMessage("Wifi error");
        tone(BUZZ, 2000);
        delay(3000);
        noTone(BUZZ);
        wifiMessageDone = true;
        return;
    }

    showMessage("Wifi ok");
    wifiMessageDone = true;
}

void loop()
{
    // Handle switch toggle
    if (toggleRequest)
    {
        toggleRequest = false;
        beepEnabled = !beepEnabled;
        if (!beepEnabled)
            noTone(BUZZ);
    }

    // Run Parola animation continuously
    if (P.displayAnimate())
        P.displayReset();

    // After WiFi message finishes scrolling, clear panel ONCE
    if (wifiMessageDone && P.displayAnimate())
    {
        P.displayClear();
        wifiMessageDone = false;
    }

    // Periodic site check (every 30s)
    if (millis() - lastCheck >= 30000)
    {
        lastCheck = millis();

        P.displayClear();
        P.displayText("PING", PA_CENTER, 0, 0, PA_PRINT, PA_PRINT);
        P.displayReset();

        bool up = checkSite();

        if (up)
        {
            lastUp = true;
            noTone(BUZZ);
            showMessage("UP");
        }
        else
        {
            lastUp = false;
            if (beepEnabled)
                tone(BUZZ, 2000);

            showMessage("Error!!!");
        }
    }
}
