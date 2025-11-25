#ifndef RAY_HPP
#define RAY_HPP

#pragma once
#include "vec3.hpp"
#include "utils.hpp"

class ray
{
private:
    point3 position; // Stores the current position of the photon
    vec3 direction;  // Stores the current direction of the photon
    bool alive;      // Whether the photon is "alive" or not

public:
    ray(point3 position, vec3 direction);

    vec3 pos() const; // returns the position of the photon
    vec3 dir() const; // returns the direction of the photon

    void propagate(double distance);                   // Propagates the photon by distance
    void reflect(point3 normal);                       // Reflects the photon about the normal
    void refract(point3 normal, double n1, double n2); // Refracrs the photon about the normal according to the two refractive indices. Also takes care of TIR.

    void kill();
    bool isAlive();
};

#endif // RAY_HPP
