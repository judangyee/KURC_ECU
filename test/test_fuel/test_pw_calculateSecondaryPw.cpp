#include <unity.h>
#include "../test_utils.h"
#include "fuel_calcs.h"
#include "config_pages.h"
#include "statuses.h"
#include "globals.h"

// Staged injection was removed from this fork (single-cylinder build with a single
// injector). calculateSecondaryPw() is now a pure pass-through: the secondary
// pulsewidth is always 0.

extern pulseWidths calculateSecondaryPw(uint16_t primaryPw, uint16_t pwLimit, uint16_t injOpenTime, const config2 &page2, const config10 &page10, const statuses &current);

struct testContext {
    config2 page2;
    config10 page10;
    statuses current;
};

static void test_calculateSecondaryPw_always_passthrough(void) {
    testContext context = {};

    auto result = calculateSecondaryPw(9000, 9000, 1000, context.page2, context.page10, context.current);

    TEST_ASSERT_EQUAL(9000, result.primary);
    TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_calculateSecondaryPw_noprimary(void) {
    testContext context = {};

    auto result = calculateSecondaryPw(0, 25500, 100, context.page2, context.page10, context.current);

    TEST_ASSERT_EQUAL(0, result.primary);
    TEST_ASSERT_EQUAL(0, result.secondary);
}

void testCalculateSecondaryPw(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calculateSecondaryPw_always_passthrough);
    RUN_TEST_P(test_calculateSecondaryPw_noprimary);
  }
}
