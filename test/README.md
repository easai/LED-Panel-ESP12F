# LED-Panel-ESP12F Unit Tests

Unit tests for the LED-Panel-ESP12F firmware using PlatformIO's Unity test framework.

## Test Files

| File | Description | Tests |
|------|-------------|-------|
| `test_state.cpp` | State management, mute toggle, WiFi state | 18 |
| `test_http_codes.cpp` | HTTP response code interpretation | 32 |
| `test_timing.cpp` | Timing calculations, millis() overflow | 27 |

## Running Tests

### On ESP12E Hardware

```bash
# Run all tests
pio test -e esp12e_test

# Run a specific test file
pio test -e esp12e_test -f test_state
pio test -e esp12e_test -f test_http_codes
pio test -e esp12e_test -f test_timing
```

### Test Output

Tests output results via Serial at 115200 baud:

```
test/test_state.cpp:45:test_state_default_values:PASS
test/test_state.cpp:67:test_should_not_check_site_when_wifi_disconnected:PASS
...
-----------------------
18 Tests 0 Failures 0 Ignored
OK
```

## Test Coverage

### State Management (`test_state.cpp`)
- ✅ Default state initialization
- ✅ Site check timing conditions
- ✅ WiFi reconnection logic
- ✅ Button debounce behavior
- ✅ Mute toggle functionality
- ✅ Message buffer validation

### HTTP Codes (`test_http_codes.cpp`)
- ✅ Connection errors (negative codes)
- ✅ 1xx Informational responses
- ✅ 2xx Success responses
- ✅ 3xx Redirect responses
- ✅ 4xx Client error responses (server is up)
- ✅ 5xx Server error responses (server is down)

### Timing (`test_timing.cpp`)
- ✅ Basic elapsed time calculations
- ✅ Millis overflow handling (uint32_t wraparound)
- ✅ Interval checking
- ✅ Timeout detection
- ✅ Debounce timing

## Test Structure

Each test file follows PlatformIO's embedded test pattern:

```cpp
#include <Arduino.h>
#include <unity.h>

void setUp(void) {
    // Runs before each test
}

void tearDown(void) {
    // Runs after each test
}

void test_example(void) {
    TEST_ASSERT_TRUE(true);
}

void setup() {
    delay(2000);  // Allow board to settle
    UNITY_BEGIN();
    RUN_TEST(test_example);
    UNITY_END();
}

void loop() {
    // Empty
}
```

## Adding New Tests

1. Create `test/test_<feature>.cpp`
2. Include `<Arduino.h>` and `<unity.h>`
3. Add `setUp()` and `tearDown()` functions
4. Write test functions prefixed with `test_`
5. Add `setup()` with `UNITY_BEGIN()`, `RUN_TEST()` calls, `UNITY_END()`
6. Add empty `loop()`

## Mock Functions

For controlled timing tests, we use mock millis:

```cpp
static uint32_t mock_millis_value = 0;
uint32_t mock_millis() { return mock_millis_value; }
void set_mock_millis(uint32_t val) { mock_millis_value = val; }
```

This allows testing timing logic without waiting for real time to pass.
