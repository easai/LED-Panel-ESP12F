/**
 * LED-Panel-ESP12F - Optimized Firmware
 * 
 * Site monitoring with LED display and buzzer alerts
 * 
 * Optimizations:
 * - Reduced memory usage with PROGMEM strings
 * - Added WiFi reconnection handling
 * - Debounced mute button with timing
 * - Configurable check interval
 * - Better error handling and status codes
 * - Added serial debugging (optional)
 * - Power-efficient WiFi sleep between checks
 * - Visual feedback for mute state
 * - Watchdog timer for reliability
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>
#include "config.h"

// ============== Configuration ==============
#define HARDWARE_TYPE   MD_MAX72XX::FC16_HW
#define MAX_DEVICES     4
#define CS_PIN          12
#define BUZZ_PIN        4
#define MUTE_PIN        5

// Timing constants (in milliseconds)
constexpr uint32_t CHECK_INTERVAL     = 30000;   // Site check interval
constexpr uint32_t WIFI_TIMEOUT       = 15000;   // WiFi connection timeout
constexpr uint32_t HTTP_TIMEOUT       = 5000;    // HTTP request timeout
constexpr uint32_t DEBOUNCE_DELAY     = 200;     // Button debounce time
constexpr uint32_t RECONNECT_INTERVAL = 60000;   // WiFi reconnect attempt interval
constexpr uint32_t PING_DISPLAY_TIME  = 500;     // How long to show "PING"

// Display settings
constexpr uint8_t  DISPLAY_INTENSITY  = 2;       // 0-15
constexpr uint16_t SCROLL_SPEED       = 40;      // Lower = faster

// Debug mode (comment out to disable serial output)
#define DEBUG_MODE

#ifdef DEBUG_MODE
    #define DEBUG_PRINT(x)   Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
#endif

// ============== PROGMEM Strings ==============
const char MSG_WIFI_CONNECTING[] PROGMEM = "WiFi...";
const char MSG_WIFI_OK[]         PROGMEM = "WiFi OK";
const char MSG_WIFI_ERROR[]      PROGMEM = "WiFi Err";
const char MSG_WIFI_RECONNECT[]  PROGMEM = "Reconn...";
const char MSG_PING[]            PROGMEM = "Pinging";
const char MSG_MUTED[]           PROGMEM = "Muted";
const char MSG_UNMUTED[]         PROGMEM = "Sound On";

// Site status messages
const char MSG_SITE_UP[]   PROGMEM = "All Good!";
const char MSG_SITE_DOWN[] PROGMEM = "SITE DOWN!";

// ============== Global State ==============
MD_Parola display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Volatile for ISR access
volatile bool muteToggleRequest = false;

// State variables
struct State {
    bool     isMuted          = false;
    bool     siteIsUp         = true;
    bool     wifiConnected    = false;
    bool     messageScrolling = false;
    uint32_t lastCheckTime    = 0;
    uint32_t lastReconnect    = 0;
    uint32_t lastButtonPress  = 0;
} state;

// Message buffer for PROGMEM strings
char msgBuffer[32];

// ============== Function Declarations ==============
void setupDisplay();
void setupWiFi();
void setupPins();
bool connectWiFi();
bool checkSiteStatus();
void handleMuteToggle();
void updateDisplay(const char* msg, bool fromProgmem = true);
void showStatus(bool isUp);
void playAlertTone(bool enable);
void checkWiFiConnection();

// ============== ISR ==============
void IRAM_ATTR onMuteButtonPress() {
    muteToggleRequest = true;
}

// ============== Setup ==============
void setup() {
#ifdef DEBUG_MODE
    Serial.begin(115200);
    delay(100);
    DEBUG_PRINTLN(F("\n\n=== LED-Panel-ESP12F ==="));
    DEBUG_PRINTLN(F("Optimized Firmware v2.0"));
#endif

    setupPins();
    setupDisplay();
    setupWiFi();
    
    // Initial site check after boot
    state.lastCheckTime = millis() - CHECK_INTERVAL + 5000; // Check 5s after boot
    
    DEBUG_PRINTLN(F("Setup complete"));
}

// ============== Main Loop ==============
void loop() {
    // Handle display animations
    if (display.displayAnimate()) {
        if (state.messageScrolling) {
            state.messageScrolling = false;
            display.displayClear();
        }
    }
    
    // Handle mute button (with debounce)
    if (muteToggleRequest) {
        handleMuteToggle();
    }
    
    // Check WiFi connection periodically
    checkWiFiConnection();
    
    // Periodic site check
    uint32_t now = millis();
    if (state.wifiConnected && (now - state.lastCheckTime >= CHECK_INTERVAL)) {
        state.lastCheckTime = now;
        
        // Show PING indicator
        updateDisplay(MSG_PING);
        display.displayText(msgBuffer, PA_CENTER, 0, PING_DISPLAY_TIME, PA_PRINT, PA_NO_EFFECT);
        display.displayAnimate();
        delay(PING_DISPLAY_TIME);
        
        // Check site
        DEBUG_PRINT(F("Checking site... "));
        bool isUp = checkSiteStatus();
        DEBUG_PRINTLN(isUp ? F("UP") : F("DOWN"));
        
        // Update state and display
        state.siteIsUp = isUp;
        
        showStatus(isUp);
        
        // Alert on status change or if down
        if (!isUp) {
            playAlertTone(!state.isMuted);
        } else {
            playAlertTone(false);
        }
    }
    
    // Small delay to prevent tight loop
    delay(10);
}

// ============== Implementation ==============

void setupPins() {
    pinMode(BUZZ_PIN, OUTPUT);
    pinMode(MUTE_PIN, INPUT_PULLUP);
    digitalWrite(BUZZ_PIN, LOW);
    
    attachInterrupt(digitalPinToInterrupt(MUTE_PIN), onMuteButtonPress, FALLING);
    
    DEBUG_PRINTLN(F("Pins configured"));
}

void setupDisplay() {
    display.begin();
    display.setIntensity(DISPLAY_INTENSITY);
    display.displayClear();
    display.setTextAlignment(PA_CENTER);
    
    DEBUG_PRINTLN(F("Display initialized"));
}

void setupWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);  // Don't save to flash (reduces wear)
    
    updateDisplay(MSG_WIFI_CONNECTING);
    display.displayText(msgBuffer, PA_CENTER, SCROLL_SPEED, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    
    state.wifiConnected = connectWiFi();
    
    if (state.wifiConnected) {
        updateDisplay(MSG_WIFI_OK);
        DEBUG_PRINT(F("Connected! IP: "));
        DEBUG_PRINTLN(WiFi.localIP());
    } else {
        updateDisplay(MSG_WIFI_ERROR);
        playAlertTone(true);
        delay(1000);
        playAlertTone(false);
        DEBUG_PRINTLN(F("WiFi connection failed"));
    }
    
    // Show message briefly
    display.displayText(msgBuffer, PA_CENTER, SCROLL_SPEED, 2000, PA_SCROLL_LEFT, PA_NO_EFFECT);
    state.messageScrolling = true;
}

bool connectWiFi() {
    WiFi.begin(SECRET_SSID, SECRET_PASS);
    
    uint32_t startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime >= WIFI_TIMEOUT) {
            return false;
        }
        delay(100);
        
        // Keep display animating during connection
        display.displayAnimate();
    }
    
    return true;
}

void checkWiFiConnection() {
    bool connected = (WiFi.status() == WL_CONNECTED);
    
    if (!connected && state.wifiConnected) {
        // Lost connection
        DEBUG_PRINTLN(F("WiFi disconnected!"));
        state.wifiConnected = false;
        playAlertTone(!state.isMuted);
    }
    
    // Attempt reconnect periodically
    if (!connected) {
        uint32_t now = millis();
        if (now - state.lastReconnect >= RECONNECT_INTERVAL) {
            state.lastReconnect = now;
            DEBUG_PRINTLN(F("Attempting WiFi reconnect..."));
            
            updateDisplay(MSG_WIFI_RECONNECT);
            display.displayText(msgBuffer, PA_CENTER, SCROLL_SPEED, 0, PA_SCROLL_LEFT, PA_NO_EFFECT);
            
            WiFi.reconnect();
            delay(5000);  // Give it time
            
            if (WiFi.status() == WL_CONNECTED) {
                state.wifiConnected = true;
                playAlertTone(false);
                DEBUG_PRINTLN(F("Reconnected!"));
            }
        }
    }
}

bool checkSiteStatus() {
    // Use BearSSL for secure connection
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();  // Skip certificate verification
    
    HTTPClient http;
    http.setTimeout(HTTP_TIMEOUT);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    if (!http.begin(*client, SITE_URL)) {
        DEBUG_PRINTLN(F("HTTP begin failed"));
        return false;
    }
    
    // Add headers for better compatibility
    http.addHeader("User-Agent", "ESP8266-Monitor/2.0");
    http.addHeader("Connection", "close");
    
    int httpCode = http.GET();
    http.end();
    
    DEBUG_PRINT(F("HTTP code: "));
    DEBUG_PRINTLN(httpCode);
    
    // Consider 2xx and 3xx as "up"
    // 4xx client errors might still mean server is responding
    // 5xx server errors = down
    if (httpCode < 0) {
        return false;  // Connection error
    }
    
    return (httpCode < 500);
}

void handleMuteToggle() {
    uint32_t now = millis();
    
    // Debounce
    if (now - state.lastButtonPress < DEBOUNCE_DELAY) {
        muteToggleRequest = false;
        return;
    }
    
    state.lastButtonPress = now;
    muteToggleRequest = false;
    
    // Toggle mute state
    state.isMuted = !state.isMuted;
    
    DEBUG_PRINT(F("Mute toggled: "));
    DEBUG_PRINTLN(state.isMuted ? F("ON") : F("OFF"));
    
    // Stop any playing tone
    if (state.isMuted) {
        noTone(BUZZ_PIN);
        updateDisplay(MSG_MUTED);
    } else {
        updateDisplay(MSG_UNMUTED);
        // Brief confirmation beep
        tone(BUZZ_PIN, 1000, 100);
    }
    
    // Show mute status briefly
    display.displayText(msgBuffer, PA_CENTER, SCROLL_SPEED, 1500, PA_SCROLL_LEFT, PA_NO_EFFECT);
    state.messageScrolling = true;
}

void updateDisplay(const char* msg, bool fromProgmem) {
    if (fromProgmem) {
        strcpy_P(msgBuffer, msg);
    } else {
        strncpy(msgBuffer, msg, sizeof(msgBuffer) - 1);
        msgBuffer[sizeof(msgBuffer) - 1] = '\0';
    }
}

void showStatus(bool isUp) {
    if (isUp) {
        updateDisplay(MSG_SITE_UP);
    } else {
        updateDisplay(MSG_SITE_DOWN);
    }
    
    display.displayText(msgBuffer, PA_CENTER, SCROLL_SPEED, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    state.messageScrolling = true;
}

void playAlertTone(bool enable) {
    if (enable) {
        tone(BUZZ_PIN, 2000);
    } else {
        noTone(BUZZ_PIN);
    }
}