#ifndef UTILS_HPP
#define UTILS_HPP

#pragma once

#include <cmath>
#include <cstdlib>
#include <limits>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

const double INF = std::numeric_limits<double>::infinity();
const double PI = 3.1415926535897932385;

enum class FieldType
{
    PLANE,
    GAUSSIAN,
    LG,
    HG,
    BLANK
};

inline double degrees_to_radians(double degrees) { return degrees * PI / 180.0; }
inline double random_double() { return std::rand() / (RAND_MAX + 1.0); }
inline double random_double(double min, double max) { return min + (max - min) * random_double(); }
inline double sq(double a) { return a * a; }
inline double factorial(int n)
{
    double res = 1.0;
    for (int i = 2; i <= n; ++i)
        res *= i;
    return res;
}

double genLaguerre(int p, int l, double x);
double hermitePol(int n, double x);

#endif