/*
    This file is part of darktable,
    Copyright (C) 2026 darktable developers.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
 * cmocka unit tests for pure math functions in common/math.h
 *
 * These tests cover the standalone utility functions that do not depend
 * on darktable global state, such as sqrf(), feqf(), interpolatef(),
 * clamp/clip helpers, fast math approximations, and vector ops.
 */
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <float.h>
#include <math.h>

#include <cmocka.h>

/* Include the header under test directly.
 * We need some darktable types that math.h depends on. */
#include "common/darktable.h"
#include "common/math.h"

#ifdef _WIN32
#include "win/main_wrapper.h"
#endif

/* epsilon for floating point comparison */
#define E 1e-6f

/*
 * TEST: sqrf — square a float
 */
static void test_sqrf(void **state)
{
  assert_float_equal(sqrf(0.0f), 0.0f, E);
  assert_float_equal(sqrf(1.0f), 1.0f, E);
  assert_float_equal(sqrf(2.0f), 4.0f, E);
  assert_float_equal(sqrf(-3.0f), 9.0f, E);
  assert_float_equal(sqrf(0.5f), 0.25f, E);
}

/*
 * TEST: sqf — also square a float (alias)
 */
static void test_sqf(void **state)
{
  assert_float_equal(sqf(0.0f), 0.0f, E);
  assert_float_equal(sqf(4.0f), 16.0f, E);
  assert_float_equal(sqf(-2.5f), 6.25f, E);
}

/*
 * TEST: feqf — float equality within epsilon
 */
static void test_feqf(void **state)
{
  /* identical values */
  assert_true(feqf(1.0f, 1.0f, E));
  assert_true(feqf(0.0f, 0.0f, E));

  /* values within epsilon */
  assert_true(feqf(1.0f, 1.0f + E / 2.0f, E));

  /* values outside epsilon */
  assert_false(feqf(1.0f, 1.0f + E * 10.0f, E));
  assert_false(feqf(0.0f, 1.0f, E));
}

/*
 * TEST: interpolatef — linear interpolation
 */
static void test_interpolatef(void **state)
{
  /* t=0 should return a */
  assert_float_equal(interpolatef(2.0f, 8.0f, 0.0f), 2.0f, E);

  /* t=1 should return b */
  assert_float_equal(interpolatef(2.0f, 8.0f, 1.0f), 8.0f, E);

  /* t=0.5 should return midpoint */
  assert_float_equal(interpolatef(0.0f, 10.0f, 0.5f), 5.0f, E);

  /* t=0.25 */
  assert_float_equal(interpolatef(0.0f, 4.0f, 0.25f), 1.0f, E);
}

/*
 * TEST: Log2 — base-2 logarithm
 */
static void test_Log2(void **state)
{
  assert_float_equal(Log2(1.0f), 0.0f, E);
  assert_float_equal(Log2(2.0f), 1.0f, E);
  assert_float_equal(Log2(4.0f), 2.0f, E);
  assert_float_equal(Log2(0.5f), -1.0f, E);
}

/*
 * TEST: dt_fast_hypotf — fast hypotenuse approximation
 * Should be reasonably close to sqrtf(x*x + y*y)
 */
static void test_dt_fast_hypotf(void **state)
{
  /* classic 3-4-5 triangle */
  float result = dt_fast_hypotf(3.0f, 4.0f);
  /* fast_hypot uses max(|x|,|y|) * sqrt(1 + (min/max)^2) approximation,
     should be within ~1% of true value */
  assert_true(fabsf(result - 5.0f) < 0.1f);

  /* zero inputs */
  assert_float_equal(dt_fast_hypotf(0.0f, 0.0f), 0.0f, E);

  /* one zero input */
  assert_float_equal(dt_fast_hypotf(5.0f, 0.0f), 5.0f, E);
  assert_float_equal(dt_fast_hypotf(0.0f, 5.0f), 5.0f, E);
}

/*
 * TEST: euclidean_norm — L2 norm of 4-channel pixel (only first 3 channels)
 */
static void test_euclidean_norm(void **state)
{
  dt_aligned_pixel_t v1 = { 1.0f, 0.0f, 0.0f, 99.0f };
  assert_float_equal(euclidean_norm(v1), 1.0f, E);

  dt_aligned_pixel_t v2 = { 3.0f, 4.0f, 0.0f, 0.0f };
  assert_float_equal(euclidean_norm(v2), 5.0f, E);

  dt_aligned_pixel_t v3 = { 0.0f, 0.0f, 0.0f, 0.0f };
  assert_float_equal(euclidean_norm(v3), 0.0f, E);
}

/*
 * TEST: dt_vector_clip — clamp pixel channels to [0, 1]
 */
static void test_dt_vector_clip(void **state)
{
  dt_aligned_pixel_t v = { -0.5f, 0.5f, 1.5f, 2.0f };
  dt_vector_clip(v);
  assert_float_equal(v[0], 0.0f, E);
  assert_float_equal(v[1], 0.5f, E);
  assert_float_equal(v[2], 1.0f, E);
  assert_float_equal(v[3], 1.0f, E);
}

/*
 * TEST: dt_vector_clipneg — clamp pixel channels to [0, inf)
 */
static void test_dt_vector_clipneg(void **state)
{
  dt_aligned_pixel_t v = { -0.5f, 0.5f, 1.5f, -999.0f };
  dt_vector_clipneg(v);
  assert_float_equal(v[0], 0.0f, E);
  assert_float_equal(v[1], 0.5f, E);
  assert_float_equal(v[2], 1.5f, E);
  assert_float_equal(v[3], 0.0f, E);
}

/*
 * TEST: dt_vector_channel_max — max of first 3 channels
 */
static void test_dt_vector_channel_max(void **state)
{
  dt_aligned_pixel_t v1 = { 0.1f, 0.9f, 0.5f, 99.0f };
  assert_float_equal(dt_vector_channel_max(v1), 0.9f, E);

  dt_aligned_pixel_t v2 = { 3.0f, 1.0f, 2.0f, 0.0f };
  assert_float_equal(dt_vector_channel_max(v2), 3.0f, E);
}

/*
 * TEST: dt_vector_add / dt_vector_sub / dt_vector_mul
 */
static void test_dt_vector_arithmetic(void **state)
{
  dt_aligned_pixel_t a = { 1.0f, 2.0f, 3.0f, 4.0f };
  dt_aligned_pixel_t b = { 5.0f, 6.0f, 7.0f, 8.0f };
  dt_aligned_pixel_t result;

  dt_vector_add(result, a, b);
  assert_float_equal(result[0], 6.0f, E);
  assert_float_equal(result[1], 8.0f, E);
  assert_float_equal(result[2], 10.0f, E);
  assert_float_equal(result[3], 12.0f, E);

  dt_vector_sub(result, b, a);
  assert_float_equal(result[0], 4.0f, E);
  assert_float_equal(result[1], 4.0f, E);
  assert_float_equal(result[2], 4.0f, E);
  assert_float_equal(result[3], 4.0f, E);

  dt_vector_mul(result, a, b);
  assert_float_equal(result[0], 5.0f, E);
  assert_float_equal(result[1], 12.0f, E);
  assert_float_equal(result[2], 21.0f, E);
  assert_float_equal(result[3], 32.0f, E);
}

/*
 * TEST: ceil_fast — fast ceiling
 */
static void test_ceil_fast(void **state)
{
  assert_float_equal(ceil_fast(2.0f), 2.0f, E);
  assert_float_equal(ceil_fast(2.1f), 3.0f, E);
  assert_float_equal(ceil_fast(2.9f), 3.0f, E);
  assert_float_equal(ceil_fast(-1.5f), -1.0f, E);
}

/*
 * TEST: mat3mulv — 3x3 matrix times 3-vector
 */
static void test_mat3mulv(void **state)
{
  /* identity matrix */
  float identity[9] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
  };
  float v[3] = { 2.0f, 3.0f, 4.0f };
  float result[3];

  mat3mulv(result, identity, v);
  assert_float_equal(result[0], 2.0f, E);
  assert_float_equal(result[1], 3.0f, E);
  assert_float_equal(result[2], 4.0f, E);

  /* scaling matrix */
  float scale[9] = {
    2.0f, 0.0f, 0.0f,
    0.0f, 3.0f, 0.0f,
    0.0f, 0.0f, 0.5f
  };
  mat3mulv(result, scale, v);
  assert_float_equal(result[0], 4.0f, E);
  assert_float_equal(result[1], 9.0f, E);
  assert_float_equal(result[2], 2.0f, E);
}

/*
 * TEST: Kahan_sum — compensated summation
 */
static void test_Kahan_sum(void **state)
{
  float error = 0.0f;
  float sum = 0.0f;

  /* sum 1.0 ten times — should be 10.0 */
  for(int i = 0; i < 10; i++)
  {
    sum = Kahan_sum(sum, &error, 1.0f);
  }
  assert_float_equal(sum, 10.0f, E);
}


int main(int argc, char *argv[])
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_sqrf),
    cmocka_unit_test(test_sqf),
    cmocka_unit_test(test_feqf),
    cmocka_unit_test(test_interpolatef),
    cmocka_unit_test(test_Log2),
    cmocka_unit_test(test_dt_fast_hypotf),
    cmocka_unit_test(test_euclidean_norm),
    cmocka_unit_test(test_dt_vector_clip),
    cmocka_unit_test(test_dt_vector_clipneg),
    cmocka_unit_test(test_dt_vector_channel_max),
    cmocka_unit_test(test_dt_vector_arithmetic),
    cmocka_unit_test(test_ceil_fast),
    cmocka_unit_test(test_mat3mulv),
    cmocka_unit_test(test_Kahan_sum),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
// clang-format off
// modelines: These editor modelines have been set for all relevant files by tools/update_modelines.py
// vim: shiftwidth=2 expandtab tabstop=2 cindent
// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-spaces modified;
// clang-format on
