/**
 * Unit Tests for LED-Panel-ESP12F
 * Test File: test_state.cpp
 * 
 * Tests for state management and logic functions
 * 
 * Run with: pio test
 */

#include <Arduino.h>
#include <unity.h>
#include <string.h>
#include <stdint.h>

// ============== Mock Definitions ==============
// These replace Arduino/ESP8266 specific types and functions for testing

#define PROGMEM
#define F(x) x
#define strcpy_P strcpy
#define pgm_read_ptr(addr) (*(addr))

// Mock millis for controlled testing
static uint32_t mock_millis_value = 0;
uint32_t mock_millis() { return mock_millis_value; }
void set_mock_millis(uint32_t val) { mock_millis_value = val; }
void advance_mock_millis(uint32_t delta) { mock_millis_value += delta; }

// ============== Constants (copied from main) ==============
constexpr uint32_t CHECK_INTERVAL     = 30000;
constexpr uint32_t DEBOUNCE_DELAY     = 200;
constexpr uint32_t RECONNECT_INTERVAL = 60000;

// ============== State Structure ==============
struct State {
    bool     isMuted          = false;
    bool     siteIsUp         = true;
    bool     wifiConnected    = false;
    bool     messageScrolling = false;
    uint32_t lastCheckTime    = 0;
    uint32_t lastReconnect    = 0;
    uint32_t lastButtonPress  = 0;
};

// Global state for tests
State state;
volatile bool muteToggleRequest = false;
char msgBuffer[32];

// ============== Test Helper Functions ==============

void resetState() {
    state = State();
    muteToggleRequest = false;
    mock_millis_value = 0;
    memset(msgBuffer, 0, sizeof(msgBuffer));
}

// ============== Logic Functions Under Test ==============

bool shouldCheckSite(uint32_t now) {
    return state.wifiConnected && (now - state.lastCheckTime >= CHECK_INTERVAL);
}

bool shouldAttemptReconnect(uint32_t now) {
    return !state.wifiConnected && (now - state.lastReconnect >= RECONNECT_INTERVAL);
}

bool isDebounced(uint32_t now) {
    return (now - state.lastButtonPress >= DEBOUNCE_DELAY);
}

void toggleMute(uint32_t now) {
    if (!isDebounced(now)) {
        return;
    }
    state.lastButtonPress = now;
    state.isMuted = !state.isMuted;
}

// ============== Tests: State Initialization ==============

void test_state_default_values(void) {
    TEST_ASSERT_FALSE(state.isMuted);
    TEST_ASSERT_TRUE(state.siteIsUp);
    TEST_ASSERT_FALSE(state.wifiConnected);
    TEST_ASSERT_FALSE(state.messageScrolling);
    TEST_ASSERT_EQUAL_UINT32(0, state.lastCheckTime);
    TEST_ASSERT_EQUAL_UINT32(0, state.lastReconnect);
    TEST_ASSERT_EQUAL_UINT32(0, state.lastButtonPress);
}

// ============== Tests: Site Check Timing ==============

void test_should_not_check_site_when_wifi_disconnected(void) {
    state.wifiConnected = false;
    state.lastCheckTime = 0;
    set_mock_millis(CHECK_INTERVAL + 1000);
    
    TEST_ASSERT_FALSE(shouldCheckSite(mock_millis()));
}

void test_should_not_check_site_before_interval(void) {
    state.wifiConnected = true;
    state.lastCheckTime = 0;
    set_mock_millis(CHECK_INTERVAL - 1);
    
    TEST_ASSERT_FALSE(shouldCheckSite(mock_millis()));
}

void test_should_check_site_after_interval(void) {
    state.wifiConnected = true;
    state.lastCheckTime = 0;
    set_mock_millis(CHECK_INTERVAL);
    
    TEST_ASSERT_TRUE(shouldCheckSite(mock_millis()));
}

void test_should_check_site_handles_millis_overflow(void) {
    state.wifiConnected = true;
    
    // Simulate millis overflow scenario
    state.lastCheckTime = 0xFFFFFFFF - 10000;  // Near max value
    set_mock_millis(20000);  // After overflow
    
    // Due to unsigned arithmetic, this should still work
    uint32_t elapsed = mock_millis() - state.lastCheckTime;
    TEST_ASSERT_TRUE(elapsed >= CHECK_INTERVAL);
}

// ============== Tests: WiFi Reconnection ==============

void test_should_not_reconnect_when_connected(void) {
    state.wifiConnected = true;
    state.lastReconnect = 0;
    set_mock_millis(RECONNECT_INTERVAL + 1000);
    
    TEST_ASSERT_FALSE(shouldAttemptReconnect(mock_millis()));
}

void test_should_not_reconnect_before_interval(void) {
    state.wifiConnected = false;
    state.lastReconnect = 0;
    set_mock_millis(RECONNECT_INTERVAL - 1);
    
    TEST_ASSERT_FALSE(shouldAttemptReconnect(mock_millis()));
}

void test_should_reconnect_after_interval(void) {
    state.wifiConnected = false;
    state.lastReconnect = 0;
    set_mock_millis(RECONNECT_INTERVAL);
    
    TEST_ASSERT_TRUE(shouldAttemptReconnect(mock_millis()));
}

// ============== Tests: Button Debounce ==============

void test_debounce_rejects_rapid_presses(void) {
    state.lastButtonPress = 0;
    set_mock_millis(DEBOUNCE_DELAY - 1);
    
    TEST_ASSERT_FALSE(isDebounced(mock_millis()));
}

void test_debounce_accepts_after_delay(void) {
    state.lastButtonPress = 0;
    set_mock_millis(DEBOUNCE_DELAY);
    
    TEST_ASSERT_TRUE(isDebounced(mock_millis()));
}

void test_debounce_accepts_long_delay(void) {
    state.lastButtonPress = 0;
    set_mock_millis(DEBOUNCE_DELAY * 10);
    
    TEST_ASSERT_TRUE(isDebounced(mock_millis()));
}

// ============== Tests: Mute Toggle ==============

void test_mute_toggle_changes_state(void) {
    TEST_ASSERT_FALSE(state.isMuted);
    
    set_mock_millis(DEBOUNCE_DELAY + 1);
    toggleMute(mock_millis());
    TEST_ASSERT_TRUE(state.isMuted);
    
    advance_mock_millis(DEBOUNCE_DELAY + 1);
    toggleMute(mock_millis());
    TEST_ASSERT_FALSE(state.isMuted);
}

void test_mute_toggle_updates_last_press_time(void) {
    set_mock_millis(5000);
    
    toggleMute(mock_millis());
    TEST_ASSERT_EQUAL_UINT32(5000, state.lastButtonPress);
}

void test_mute_toggle_rejected_when_debouncing(void) {
    state.lastButtonPress = 100;
    set_mock_millis(100 + DEBOUNCE_DELAY - 1);
    
    bool initialMuted = state.isMuted;
    toggleMute(mock_millis());
    
    TEST_ASSERT_EQUAL(initialMuted, state.isMuted);  // Should not change
}

// ============== Tests: Site Status ==============

void test_site_status_initially_up(void) {
    TEST_ASSERT_TRUE(state.siteIsUp);
}

void test_site_status_can_be_set_down(void) {
    state.siteIsUp = false;
    TEST_ASSERT_FALSE(state.siteIsUp);
}

// ============== Tests: Message Buffer ==============

void test_message_buffer_size(void) {
    TEST_ASSERT_EQUAL(32, sizeof(msgBuffer));
}

void test_message_buffer_can_hold_longest_message(void) {
    const char* longestMsg = "SITE DOWN!";  // 10 chars + null
    TEST_ASSERT_TRUE(strlen(longestMsg) < sizeof(msgBuffer));
}

// ============== Unity Setup/Teardown ==============

void setUp(void) {
    resetState();
}

void tearDown(void) {
    // Clean up after each test
}

// ============== Test Runner ==============

void setup() {
    delay(2000);  // Allow board to settle
    
    UNITY_BEGIN();
    
    // State initialization tests
    RUN_TEST(test_state_default_values);
    
    // Site check timing tests
    RUN_TEST(test_should_not_check_site_when_wifi_disconnected);
    RUN_TEST(test_should_not_check_site_before_interval);
    RUN_TEST(test_should_check_site_after_interval);
    RUN_TEST(test_should_check_site_handles_millis_overflow);
    
    // WiFi reconnection tests
    RUN_TEST(test_should_not_reconnect_when_connected);
    RUN_TEST(test_should_not_reconnect_before_interval);
    RUN_TEST(test_should_reconnect_after_interval);
    
    // Button debounce tests
    RUN_TEST(test_debounce_rejects_rapid_presses);
    RUN_TEST(test_debounce_accepts_after_delay);
    RUN_TEST(test_debounce_accepts_long_delay);
    
    // Mute toggle tests
    RUN_TEST(test_mute_toggle_changes_state);
    RUN_TEST(test_mute_toggle_updates_last_press_time);
    RUN_TEST(test_mute_toggle_rejected_when_debouncing);
    
    // Site status tests
    RUN_TEST(test_site_status_initially_up);
    RUN_TEST(test_site_status_can_be_set_down);
    
    // Message buffer tests
    RUN_TEST(test_message_buffer_size);
    RUN_TEST(test_message_buffer_can_hold_longest_message);
    
    UNITY_END();
}

void loop() {
    // Nothing to do here
}
