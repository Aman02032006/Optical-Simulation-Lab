#ifndef OPTICAL_ELEMENT_HPP
#define OPTICAL_ELEMENT_HPP

#pragma once

#include "vec3.hpp"
#include "ray.hpp"
#include "wavefront.hpp"
#include "utils.hpp"
#include <string>
#include <memory>

class OpticalElement
{
private:
    vec3 position;    // Stores the position of the element
    vec3 orientation; // Stores the Orientation of the element
    std::string name; // Stores the ID of the element

public:
    vec3 u, v, w; // Stores the 3 orthogonal vectors of the Local frame determined by the orientation
    OpticalElement(const vec3 &position, const vec3 &orientation, const std::string &name);
    virtual ~OpticalElement() = default;

    vec3 getPosition() const;
    vec3 getOrientation() const;
    std::string getName() const;

    virtual void setPosition(vec3 pos);
    virtual void setOrientation(vec3 o);

    void init_local_frame();

    virtual double hit(const ray &beamlet) = 0;
    virtual void interact_ray(ray &beamlet) = 0;
    virtual void interact_wavefront(WaveFront &A) = 0;
    virtual void reset() = 0;
};

#endif