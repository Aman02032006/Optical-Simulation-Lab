#ifndef OPTICAL_ELEMENT_H
#define OPTICAL_ELEMENT_H

#pragma once

#include "vec3.hpp"
#include "ray.hpp"
#include "wavefront.hpp"
#include <string>
#include <memory>

class OpticalElement
{
protected:
    vec3 position;    // Stores the position of the element
    vec3 orientation; // Stores the Orientation of the element
    std::string name; // Stores the ID of the element

public:
    // Constructor
    OpticalElement(const vec3 &position, const vec3 &orientation, const std::string &name);

    // Virtual Destructor
    virtual ~OpticalElement() = default;

    // Getters
    vec3 getPosition() const;
    vec3 getOrientation() const;
    std::string getName() const;

    // Abstract Methods
    virtual double hit(const ray &beamlet) = 0;
    virtual void interact_ray(ray &beamlet) = 0;
    virtual void interact_wavefront(WaveFront &A) = 0;
};

#endif