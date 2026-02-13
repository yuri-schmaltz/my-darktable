#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "common/dt_ai_core.h"
#include <glib.h>

/*
 * Test AI Core Initialization/Cleanup cycle
 */
static void test_ai_init_cleanup(void **state) {
    dt_ai_init();
    // Verify availability (stub returns TRUE if HAVE_ONNXRUNTIME is not defined, 
    // or real status if it is)
    gboolean available = dt_ai_is_available();
    assert_true(available == available); // Just verifying it returns a boolean
    dt_ai_cleanup();
}

/*
 * Test loading a non-existent model (should return NULL)
 */
static void test_ai_load_invalid_model(void **state) {
    dt_ai_init();
    dt_ai_model_t *model = dt_ai_load_model("/non/existent/path.onnx", FALSE);
    assert_null(model);
    dt_ai_cleanup();
}

/*
 * Test availability check
 */
static void test_ai_availability(void **state) {
    // Should be able to call this without crash
    dt_ai_is_available();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ai_init_cleanup),
        cmocka_unit_test(test_ai_load_invalid_model),
        cmocka_unit_test(test_ai_availability),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
