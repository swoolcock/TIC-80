
#include "lily.h"
#define LILY_NO_EXPORT
#include "../../vendor/lily/src/lily.h"
#include "lily_pkg_tic80_bindings.h"

void lily_tic80__cls(lily_state *s)
{
    lily_return_none(s);
}

// void lily_tic80__abs(lily_state *s)
// {
//     int64_t x = lily_arg_integer(s, 0);
//
//     lily_return_integer(s, llabs(x));
// }
//
// void lily_tic80__acos(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, acos(x));
// }
//
// void lily_tic80__acosh(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, acosh(x));
// }
//
// void lily_tic80__asin(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, asin(x));
// }
//
// void lily_tic80__asinh(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, asinh(x));
// }
//
// void lily_tic80__atan(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, atan(x));
// }
//
// void lily_tic80__atanh(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, atanh(x));
// }
//
// void lily_tic80__atan2(lily_state *s)
// {
//     double y = lily_arg_double(s, 0);
//     double x = lily_arg_double(s, 1);
//
//     lily_return_double(s, atan2(y, x));
// }
//
// void lily_tic80__cbrt(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, cbrt(x));
// }
//
// void lily_tic80__ceil(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, ceil(x));
// }
//
// void lily_tic80__cos(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, cos(x));
// }
//
// void lily_tic80__cosh(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, cosh(x));
// }
//
// void lily_tic80__exp(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, exp(x));
// }
//
// void lily_tic80__exp2(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, exp2(x));
// }
//
// void lily_tic80__fabs(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, fabs(x));
// }
//
// void lily_tic80__floor(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, floor(x));
// }
//
// void lily_tic80__fmod(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//     double y = lily_arg_double(s, 1);
//
//     lily_return_double(s, fmod(x, y));
// }
//
// void lily_tic80__hypot(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//     double y = lily_arg_double(s, 1);
//
//     lily_return_double(s, hypot(x, y));
// }
//
// void lily_tic80__is_infinity(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_boolean(s, isinf(x));
// }
//
// void lily_tic80__is_nan(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_boolean(s, isnan(x));
// }
//
// void lily_tic80__ldexp(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//     int y = (int)lily_arg_integer(s, 1);
//
//     lily_return_double(s, ldexp(x, y));
// }
//
// void lily_tic80__log(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, log(x));
// }
//
// void lily_tic80__log2(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, log2(x));
// }
//
// void lily_tic80__log10(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, log10(x));
// }
//
// void lily_tic80__modf(lily_state *s)
// {
//     double i, f;
//     double x = lily_arg_double(s, 0);
//
//     f = modf(x, &i);
//
//     lily_container_val *tpl = lily_push_tuple(s, 2);
//     lily_push_double(s, i);
//     lily_con_set_from_stack(s, tpl, 0);
//     lily_push_double(s, f);
//     lily_con_set_from_stack(s, tpl, 1);
//
//     lily_return_top(s);
// }
//
// void lily_tic80__pow(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//     double y = lily_arg_double(s, 1);
//
//     lily_return_double(s, pow(x, y));
// }
//
// void lily_tic80__round(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, round(x));
// }
//
// void lily_tic80__sin(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, sin(x));
// }
//
// void lily_tic80__sinh(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, sinh(x));
// }
//
// void lily_tic80__sqrt(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, sqrt(x));
// }
//
// void lily_tic80__tan(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, tan(x));
// }
//
// void lily_tic80__tanh(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, tanh(x));
// }
//
// void lily_tic80__to_deg(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, x * (180 / M_PI));
// }
//
// void lily_tic80__to_rad(lily_state *s)
// {
//     double x = lily_arg_double(s, 0);
//
//     lily_return_double(s, x * (M_PI / 180));
// }
//
// void lily_tic80_constant_huge(lily_state *s)
// {
//     lily_push_double(s, HUGE_VAL);
// }
//
// void lily_tic80_constant_infinity(lily_state *s)
// {
//     lily_push_double(s, INFINITY);
// }
//
// void lily_tic80_constant_nan(lily_state *s)
// {
//     lily_push_double(s, NAN);
// }
//
// void lily_tic80_constant_pi(lily_state *s)
// {
//     lily_push_double(s, M_PI);
// }

LILY_DECLARE_TIC80_CALL_TABLE
