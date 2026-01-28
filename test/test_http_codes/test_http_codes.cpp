/**
 * Unit Tests for LED-Panel-ESP12F
 * Test File: test_http_codes.cpp
 * 
 * Tests for HTTP response code interpretation
 * 
 * Run with: pio test
 */

#include <Arduino.h>
#include <unity.h>
#include <stdint.h>

// ============== HTTP Code Interpretation Logic ==============
// Mirrors the logic in checkSiteStatus()

/**
 * Determines if site is "up" based on HTTP response code
 * 
 * - Negative codes: Connection errors (down)
 * - 1xx: Informational (up)
 * - 2xx: Success (up)
 * - 3xx: Redirect (up - server responding)
 * - 4xx: Client error (up - server responding)
 * - 5xx: Server error (down)
 */
bool isSiteUp(int httpCode) {
    if (httpCode < 0) {
        return false;  // Connection error
    }
    return (httpCode < 500);
}

/**
 * Get status description for logging
 */
const char* getStatusDescription(int httpCode) {
    if (httpCode < 0) return "Connection Error";
    if (httpCode < 200) return "Informational";
    if (httpCode < 300) return "Success";
    if (httpCode < 400) return "Redirect";
    if (httpCode < 500) return "Client Error";
    return "Server Error";
}

// ============== Tests: Connection Errors ==============

void test_connection_error_minus_one(void) {
    TEST_ASSERT_FALSE(isSiteUp(-1));
}

void test_connection_error_timeout(void) {
    // ESP8266HTTPClient returns -1 for timeout
    TEST_ASSERT_FALSE(isSiteUp(-1));
}

void test_connection_error_refused(void) {
    // Connection refused
    TEST_ASSERT_FALSE(isSiteUp(-2));
}

void test_connection_error_dns_failed(void) {
    // DNS lookup failed
    TEST_ASSERT_FALSE(isSiteUp(-3));
}

void test_connection_error_large_negative(void) {
    TEST_ASSERT_FALSE(isSiteUp(-100));
}

// ============== Tests: 1xx Informational ==============

void test_http_100_continue(void) {
    TEST_ASSERT_TRUE(isSiteUp(100));
}

void test_http_101_switching_protocols(void) {
    TEST_ASSERT_TRUE(isSiteUp(101));
}

// ============== Tests: 2xx Success ==============

void test_http_200_ok(void) {
    TEST_ASSERT_TRUE(isSiteUp(200));
}

void test_http_201_created(void) {
    TEST_ASSERT_TRUE(isSiteUp(201));
}

void test_http_204_no_content(void) {
    TEST_ASSERT_TRUE(isSiteUp(204));
}

void test_http_299_edge_case(void) {
    TEST_ASSERT_TRUE(isSiteUp(299));
}

// ============== Tests: 3xx Redirect ==============

void test_http_301_moved_permanently(void) {
    TEST_ASSERT_TRUE(isSiteUp(301));
}

void test_http_302_found(void) {
    TEST_ASSERT_TRUE(isSiteUp(302));
}

void test_http_304_not_modified(void) {
    TEST_ASSERT_TRUE(isSiteUp(304));
}

void test_http_307_temporary_redirect(void) {
    TEST_ASSERT_TRUE(isSiteUp(307));
}

void test_http_308_permanent_redirect(void) {
    TEST_ASSERT_TRUE(isSiteUp(308));
}

// ============== Tests: 4xx Client Errors ==============
// Note: 4xx errors mean server IS responding, so site is "up"

void test_http_400_bad_request(void) {
    TEST_ASSERT_TRUE(isSiteUp(400));
}

void test_http_401_unauthorized(void) {
    TEST_ASSERT_TRUE(isSiteUp(401));
}

void test_http_403_forbidden(void) {
    TEST_ASSERT_TRUE(isSiteUp(403));
}

void test_http_404_not_found(void) {
    TEST_ASSERT_TRUE(isSiteUp(404));
}

void test_http_429_too_many_requests(void) {
    TEST_ASSERT_TRUE(isSiteUp(429));
}

void test_http_499_edge_case(void) {
    TEST_ASSERT_TRUE(isSiteUp(499));
}

// ============== Tests: 5xx Server Errors ==============

void test_http_500_internal_server_error(void) {
    TEST_ASSERT_FALSE(isSiteUp(500));
}

void test_http_501_not_implemented(void) {
    TEST_ASSERT_FALSE(isSiteUp(501));
}

void test_http_502_bad_gateway(void) {
    TEST_ASSERT_FALSE(isSiteUp(502));
}

void test_http_503_service_unavailable(void) {
    TEST_ASSERT_FALSE(isSiteUp(503));
}

void test_http_504_gateway_timeout(void) {
    TEST_ASSERT_FALSE(isSiteUp(504));
}

void test_http_599_edge_case(void) {
    TEST_ASSERT_FALSE(isSiteUp(599));
}

// ============== Tests: Edge Cases ==============

void test_http_0_is_treated_as_up(void) {
    // Zero is technically >= 0 and < 500
    TEST_ASSERT_TRUE(isSiteUp(0));
}

void test_large_success_code(void) {
    // Unusual but valid
    TEST_ASSERT_TRUE(isSiteUp(250));
}

// ============== Tests: Status Descriptions ==============

void test_status_description_connection_error(void) {
    TEST_ASSERT_EQUAL_STRING("Connection Error", getStatusDescription(-1));
}

void test_status_description_success(void) {
    TEST_ASSERT_EQUAL_STRING("Success", getStatusDescription(200));
}

void test_status_description_redirect(void) {
    TEST_ASSERT_EQUAL_STRING("Redirect", getStatusDescription(301));
}

void test_status_description_client_error(void) {
    TEST_ASSERT_EQUAL_STRING("Client Error", getStatusDescription(404));
}

void test_status_description_server_error(void) {
    TEST_ASSERT_EQUAL_STRING("Server Error", getStatusDescription(500));
}

// ============== Unity Setup/Teardown ==============

void setUp(void) {
    // Nothing to set up
}

void tearDown(void) {
    // Nothing to tear down
}

// ============== Test Runner ==============

void setup() {
    delay(2000);  // Allow board to settle
    
    UNITY_BEGIN();
    
    // Connection error tests
    RUN_TEST(test_connection_error_minus_one);
    RUN_TEST(test_connection_error_timeout);
    RUN_TEST(test_connection_error_refused);
    RUN_TEST(test_connection_error_dns_failed);
    RUN_TEST(test_connection_error_large_negative);
    
    // 1xx tests
    RUN_TEST(test_http_100_continue);
    RUN_TEST(test_http_101_switching_protocols);
    
    // 2xx tests
    RUN_TEST(test_http_200_ok);
    RUN_TEST(test_http_201_created);
    RUN_TEST(test_http_204_no_content);
    RUN_TEST(test_http_299_edge_case);
    
    // 3xx tests
    RUN_TEST(test_http_301_moved_permanently);
    RUN_TEST(test_http_302_found);
    RUN_TEST(test_http_304_not_modified);
    RUN_TEST(test_http_307_temporary_redirect);
    RUN_TEST(test_http_308_permanent_redirect);
    
    // 4xx tests
    RUN_TEST(test_http_400_bad_request);
    RUN_TEST(test_http_401_unauthorized);
    RUN_TEST(test_http_403_forbidden);
    RUN_TEST(test_http_404_not_found);
    RUN_TEST(test_http_429_too_many_requests);
    RUN_TEST(test_http_499_edge_case);
    
    // 5xx tests
    RUN_TEST(test_http_500_internal_server_error);
    RUN_TEST(test_http_501_not_implemented);
    RUN_TEST(test_http_502_bad_gateway);
    RUN_TEST(test_http_503_service_unavailable);
    RUN_TEST(test_http_504_gateway_timeout);
    RUN_TEST(test_http_599_edge_case);
    
    // Edge case tests
    RUN_TEST(test_http_0_is_treated_as_up);
    RUN_TEST(test_large_success_code);
    
    // Status description tests
    RUN_TEST(test_status_description_connection_error);
    RUN_TEST(test_status_description_success);
    RUN_TEST(test_status_description_redirect);
    RUN_TEST(test_status_description_client_error);
    RUN_TEST(test_status_description_server_error);
    
    UNITY_END();
}

void loop() {
    // Nothing to do here
}
