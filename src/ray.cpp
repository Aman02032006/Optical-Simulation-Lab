#include "ray.hpp"
#include <cmath>

ray::ray(point3 position, vec3 direction)
    : position{position}, direction{direction}, alive{true} {}

vec3 ray::pos() const
{
    return position;
}

vec3 ray::dir() const
{
    return direction;
}

void ray::propagate(double distance)
{
    position += direction * distance;
}

void ray::reflect(point3 normal)
{
    direction -= 2 * normal * (dot(direction, normal));
}

void ray::refract(point3 normal, double n1, double n2)
{
    double a = 1 - (sq(n1 / n2)) * (1 - sq(dot(direction, normal)));

    if (a < 0.0)
        reflect(normal); // Total internal reflection
    else
        direction = (n1 / n2) * (direction - dot(direction, normal) * normal) - normal * sqrt(a);
}

void ray::kill()
{
    alive = false;
}

bool ray::isAlive()
{
    return alive;
}
