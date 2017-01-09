#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <limits>
static const double INFI  = std::numeric_limits<double>::max();
static const double EPS  = std::numeric_limits<double>::epsilon();

#ifndef M_PI
const double PI = 3.14159;
#else
const double PI = M_PI;
#endif


double inline max(double a, double b) {
    return ( a > b ) ? a:b;
}