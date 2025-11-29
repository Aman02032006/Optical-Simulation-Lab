#ifndef UTILS_HPP
#define UTILS_HPP

#pragma once

#include <cmath>
#include <cstdlib>
#include <limits>

// --- Constants ---
const double INF = std::numeric_limits<double>::infinity();
const double PI = 3.1415926535897932385;

// --- Enums ---
enum class FieldType
{
    PLANE,
    GAUSSIAN,
    LG,
    HG
};

// --- Inline Utility Functions ---
// (These must stay in the header to be inlined)

inline double degrees_to_radians(double degrees)
{
    return degrees * PI / 180.0;
}

// Generates random doubles in [0.0, 1.0)
inline double random_double()
{
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max)
{
    return min + (max - min) * random_double();
}

inline double sq(double a)
{
    return a * a;
}

inline double factorial(int n)
{
    double res = 1.0;
    for (int i = 2; i <= n; ++i)
        res *= i;
    return res;
}

// --- Function Declarations ---
// (Definitions moved to utils.cpp to fix Linker Errors)

double genLaguerre(int p, int l, double x);
double hermitePol(int n, double x);

#endif // UTILS_HPP