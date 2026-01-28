/**
 * Unit Tests for LED-Panel-ESP12F
 * Test File: test_timing.cpp
 * 
 * Tests for timing calculations and millis() overflow handling
 * 
 * Run with: pio test
 */

#include <Arduino.h>
#include <unity.h>
#include <stdint.h>

// ============== Mock millis for controlled testing ==============
static uint32_t mock_millis_value = 0;
uint32_t mock_millis() { return mock_millis_value; }
void set_mock_millis(uint32_t val) { mock_millis_value = val; }

// ============== Constants ==============
constexpr uint32_t CHECK_INTERVAL     = 30000;   // 30 seconds
constexpr uint32_t DEBOUNCE_DELAY     = 200;     // 200 ms
constexpr uint32_t RECONNECT_INTERVAL = 60000;   // 60 seconds
constexpr uint32_t WIFI_TIMEOUT       = 15000;   // 15 seconds

// ============== Timing Functions ==============

/**
 * Calculate elapsed time handling overflow correctly
 * Due to unsigned arithmetic, this works even when millis wraps
 */
uint32_t elapsedTime(uint32_t startTime, uint32_t currentTime) {
    return currentTime - startTime;
}

/**
 * Check if interval has passed
 */
bool intervalPassed(uint32_t lastTime, uint32_t interval) {
    return elapsedTime(lastTime, mock_millis()) >= interval;
}

/**
 * Check if timeout occurred
 */
bool hasTimedOut(uint32_t startTime, uint32_t timeout) {
    return elapsedTime(startTime, mock_millis()) >= timeout;
}

// ============== Tests: Basic Elapsed Time ==============

void test_elapsed_time_zero(void) {
    TEST_ASSERT_EQUAL_UINT32(0, elapsedTime(1000, 1000));
}

void test_elapsed_time_simple(void) {
    TEST_ASSERT_EQUAL_UINT32(500, elapsedTime(1000, 1500));
}

void test_elapsed_time_large_values(void) {
    TEST_ASSERT_EQUAL_UINT32(1000000, elapsedTime(0, 1000000));
}

// ============== Tests: Millis Overflow ==============

void test_elapsed_time_at_overflow_boundary(void) {
    uint32_t start = 0xFFFFFFFF - 100;  // 100ms before overflow
    uint32_t current = 100;              // 100ms after overflow
    
    // Should be 200ms elapsed (100 + 100)
    TEST_ASSERT_EQUAL_UINT32(200, elapsedTime(start, current));
}

void test_elapsed_time_just_before_overflow(void) {
    uint32_t start = 0xFFFFFFFE;
    uint32_t current = 0xFFFFFFFF;
    
    TEST_ASSERT_EQUAL_UINT32(1, elapsedTime(start, current));
}

void test_elapsed_time_overflow_with_large_delta(void) {
    uint32_t start = 0xFFFFFFFF - 10000;  // 10s before overflow
    uint32_t current = 20000;              // 20s after overflow
    
    // Should be 30000ms (10000 + 20000)
    TEST_ASSERT_EQUAL_UINT32(30000, elapsedTime(start, current));
}

void test_check_interval_across_overflow(void) {
    uint32_t lastCheck = 0xFFFFFFFF - 15000;  // 15s before overflow
    set_mock_millis(15001);  // Just after CHECK_INTERVAL would pass
    
    TEST_ASSERT_TRUE(intervalPassed(lastCheck, CHECK_INTERVAL));
}

// ============== Tests: Interval Checking ==============

void test_interval_not_passed_immediately(void) {
    set_mock_millis(1000);
    TEST_ASSERT_FALSE(intervalPassed(1000, CHECK_INTERVAL));
}

void test_interval_not_passed_just_before(void) {
    set_mock_millis(CHECK_INTERVAL - 1);
    TEST_ASSERT_FALSE(intervalPassed(0, CHECK_INTERVAL));
}

void test_interval_passed_exactly(void) {
    set_mock_millis(CHECK_INTERVAL);
    TEST_ASSERT_TRUE(intervalPassed(0, CHECK_INTERVAL));
}

void test_interval_passed_after(void) {
    set_mock_millis(CHECK_INTERVAL + 1000);
    TEST_ASSERT_TRUE(intervalPassed(0, CHECK_INTERVAL));
}

void test_reconnect_interval(void) {
    set_mock_millis(RECONNECT_INTERVAL);
    TEST_ASSERT_TRUE(intervalPassed(0, RECONNECT_INTERVAL));
}

// ============== Tests: Timeout ==============

void test_timeout_not_reached(void) {
    set_mock_millis(WIFI_TIMEOUT - 1);
    TEST_ASSERT_FALSE(hasTimedOut(0, WIFI_TIMEOUT));
}

void test_timeout_reached_exactly(void) {
    set_mock_millis(WIFI_TIMEOUT);
    TEST_ASSERT_TRUE(hasTimedOut(0, WIFI_TIMEOUT));
}

void test_timeout_exceeded(void) {
    set_mock_millis(WIFI_TIMEOUT + 5000);
    TEST_ASSERT_TRUE(hasTimedOut(0, WIFI_TIMEOUT));
}

void test_timeout_with_offset_start(void) {
    uint32_t startTime = 10000;
    set_mock_millis(startTime + WIFI_TIMEOUT);
    TEST_ASSERT_TRUE(hasTimedOut(startTime, WIFI_TIMEOUT));
}

// ============== Tests: Debounce Timing ==============

void test_debounce_immediate_reject(void) {
    set_mock_millis(0);
    TEST_ASSERT_FALSE(intervalPassed(0, DEBOUNCE_DELAY));
}

void test_debounce_too_soon(void) {
    set_mock_millis(DEBOUNCE_DELAY - 1);
    TEST_ASSERT_FALSE(intervalPassed(0, DEBOUNCE_DELAY));
}

void test_debounce_exact(void) {
    set_mock_millis(DEBOUNCE_DELAY);
    TEST_ASSERT_TRUE(intervalPassed(0, DEBOUNCE_DELAY));
}

void test_debounce_after_delay(void) {
    set_mock_millis(DEBOUNCE_DELAY + 100);
    TEST_ASSERT_TRUE(intervalPassed(0, DEBOUNCE_DELAY));
}

// ============== Tests: Multiple Intervals ==============

void test_multiple_check_intervals(void) {
    uint32_t lastCheck = 0;
    
    // First interval
    set_mock_millis(CHECK_INTERVAL);
    TEST_ASSERT_TRUE(intervalPassed(lastCheck, CHECK_INTERVAL));
    lastCheck = mock_millis();
    
    // Second interval
    set_mock_millis(CHECK_INTERVAL * 2);
    TEST_ASSERT_TRUE(intervalPassed(lastCheck, CHECK_INTERVAL));
    lastCheck = mock_millis();
    
    // Third interval
    set_mock_millis(CHECK_INTERVAL * 3);
    TEST_ASSERT_TRUE(intervalPassed(lastCheck, CHECK_INTERVAL));
}

void test_multiple_intervals_not_ready(void) {
    uint32_t lastCheck = CHECK_INTERVAL;
    set_mock_millis(CHECK_INTERVAL + CHECK_INTERVAL / 2);  // Half interval later
    
    TEST_ASSERT_FALSE(intervalPassed(lastCheck, CHECK_INTERVAL));
}

// ============== Tests: Constants Validation ==============

void test_check_interval_value(void) {
    TEST_ASSERT_EQUAL_UINT32(30000, CHECK_INTERVAL);
}

void test_debounce_delay_value(void) {
    TEST_ASSERT_EQUAL_UINT32(200, DEBOUNCE_DELAY);
}

void test_reconnect_interval_value(void) {
    TEST_ASSERT_EQUAL_UINT32(60000, RECONNECT_INTERVAL);
}

void test_wifi_timeout_value(void) {
    TEST_ASSERT_EQUAL_UINT32(15000, WIFI_TIMEOUT);
}

void test_reconnect_longer_than_check(void) {
    TEST_ASSERT_TRUE(RECONNECT_INTERVAL > CHECK_INTERVAL);
}

// ============== Unity Setup/Teardown ==============

void setUp(void) {
    mock_millis_value = 0;
}

void tearDown(void) {
    // Nothing to tear down
}

// ============== Test Runner ==============

void setup() {
    delay(2000);  // Allow board to settle
    
    UNITY_BEGIN();
    
    // Basic elapsed time tests
    RUN_TEST(test_elapsed_time_zero);
    RUN_TEST(test_elapsed_time_simple);
    RUN_TEST(test_elapsed_time_large_values);
    
    // Overflow tests
    RUN_TEST(test_elapsed_time_at_overflow_boundary);
    RUN_TEST(test_elapsed_time_just_before_overflow);
    RUN_TEST(test_elapsed_time_overflow_with_large_delta);
    RUN_TEST(test_check_interval_across_overflow);
    
    // Interval checking tests
    RUN_TEST(test_interval_not_passed_immediately);
    RUN_TEST(test_interval_not_passed_just_before);
    RUN_TEST(test_interval_passed_exactly);
    RUN_TEST(test_interval_passed_after);
    RUN_TEST(test_reconnect_interval);
    
    // Timeout tests
    RUN_TEST(test_timeout_not_reached);
    RUN_TEST(test_timeout_reached_exactly);
    RUN_TEST(test_timeout_exceeded);
    RUN_TEST(test_timeout_with_offset_start);
    
    // Debounce tests
    RUN_TEST(test_debounce_immediate_reject);
    RUN_TEST(test_debounce_too_soon);
    RUN_TEST(test_debounce_exact);
    RUN_TEST(test_debounce_after_delay);
    
    // Multiple intervals tests
    RUN_TEST(test_multiple_check_intervals);
    RUN_TEST(test_multiple_intervals_not_ready);
    
    // Constants validation tests
    RUN_TEST(test_check_interval_value);
    RUN_TEST(test_debounce_delay_value);
    RUN_TEST(test_reconnect_interval_value);
    RUN_TEST(test_wifi_timeout_value);
    RUN_TEST(test_reconnect_longer_than_check);
    
    UNITY_END();
}

void loop() {
    // Nothing to do here
}
