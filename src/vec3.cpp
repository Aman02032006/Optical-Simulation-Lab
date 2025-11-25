#include "vec3.hpp"

// --- Class methods ---

vec3::vec3() : e{0, 0, 0} {}

vec3::vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

double vec3::x() const { return e[0]; }
double vec3::y() const { return e[1]; }
double vec3::z() const { return e[2]; }

vec3 vec3::operator-() const { return vec3(-e[0], -e[1], -e[2]); }
double vec3::operator[](int i) const { return e[i]; }
double &vec3::operator[](int i) { return e[i]; }

vec3 &vec3::operator+=(const vec3 &v)
{
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
}

vec3 &vec3::operator-=(const vec3 &v)
{
    e[0] -= v.e[0];
    e[1] -= v.e[1];
    e[2] -= v.e[2];
    return *this;
}

vec3 &vec3::operator*=(double t)
{
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
}

vec3 &vec3::operator/=(double t)
{
    return *this *= 1 / t;
}

vec3 &vec3::operator=(const vec3 &v)
{
    e[0] = v.x();
    e[1] = v.y();
    e[2] = v.z();
    return *this;
}

double vec3::length() const
{
    return sqrt(length_squared());
}

double vec3::length_squared() const
{
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
}

bool vec3::near_zero() const
{
    auto s = 1e-8;
    return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
}

vec3 vec3::random()
{
    return vec3(random_double(), random_double(), random_double());
}

vec3 vec3::random(double min, double max)
{
    return vec3(
        random_double(min, max),
        random_double(min, max),
        random_double(min, max));
}

// --- Free utility functions ---

std::ostream &operator<<(std::ostream &out, const vec3 &v)
{
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

vec3 operator+(const vec3 &u, const vec3 &v)
{
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

vec3 operator-(const vec3 &u, const vec3 &v)
{
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

vec3 operator*(const vec3 &u, const vec3 &v)
{
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

vec3 operator*(double t, const vec3 &v)
{
    return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

vec3 operator*(const vec3 &v, double t)
{
    return t * v;
}

vec3 operator/(const vec3 &v, double t)
{
    return (1 / t) * v;
}

double dot(const vec3 &u, const vec3 &v)
{
    return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2];
}

vec3 cross(const vec3 &u, const vec3 &v)
{
    return vec3(
        u.e[1] * v.e[2] - u.e[2] * v.e[1],
        u.e[2] * v.e[0] - u.e[0] * v.e[2],
        u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

vec3 unit_vector(const vec3 &v)
{
    return v / v.length();
}

vec3 random_unit_vector()
{
    while (true)
    {
        auto p = vec3::random(-1, 1);
        auto lensq = p.length_squared();
        if (1e-160 < lensq && lensq <= 1)
            return p / sqrt(lensq);
    }
}

vec3 random_on_hemisphere(const vec3 &normal)
{
    vec3 on_unit_sphere = random_unit_vector();
    return (dot(on_unit_sphere, normal) > 0.0) ? on_unit_sphere : -on_unit_sphere;
}
