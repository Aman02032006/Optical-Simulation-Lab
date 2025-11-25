#ifndef VEC3_H
#define VEC3_H

#pragma once
#include <iostream>
#include <cmath>
#include "utils.hpp"

// Avoid polluting global namespace with `using namespace std;`

class vec3
{
public:
    double e[3]; // array of size 3 to act as the vector

    // Constructors
    vec3();
    vec3(double e0, double e1, double e2);

    // Component accessors
    double x() const;
    double y() const;
    double z() const;

    // Operator overloads
    vec3 operator-() const;
    double operator[](int i) const;
    double &operator[](int i);

    vec3 &operator+=(const vec3 &v);
    vec3 &operator-=(const vec3 &v);
    vec3 &operator*=(double t);
    vec3 &operator/=(double t);
    vec3 &operator=(const vec3 &v);

    // Length computations
    double length() const;
    double length_squared() const;
    bool near_zero() const;

    // Random vector functions (declarations only)
    static vec3 random();
    static vec3 random(double min, double max);
};

// Type alias
using point3 = vec3;

// Utility functions
std::ostream &operator<<(std::ostream &out, const vec3 &v);
vec3 operator+(const vec3 &u, const vec3 &v);
vec3 operator-(const vec3 &u, const vec3 &v);
vec3 operator*(const vec3 &u, const vec3 &v);
vec3 operator*(double t, const vec3 &v);
vec3 operator*(const vec3 &v, double t);
vec3 operator/(const vec3 &v, double t);

double dot(const vec3 &u, const vec3 &v);
vec3 cross(const vec3 &u, const vec3 &v);
vec3 unit_vector(const vec3 &v);

vec3 random_unit_vector();
vec3 random_on_hemisphere(const vec3 &normal);

#endif // VEC3_H
