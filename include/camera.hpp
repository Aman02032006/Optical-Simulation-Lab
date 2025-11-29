#ifndef CAMERA_HPP
#define CAMERA_HPP

#pragma once

#include "vec3.hpp"
#include "ray.hpp"
#include "wavefront.hpp"
#include "optical_element.hpp"
#include "matplot/matplot.h"
#include <string>
#include <memory>

class Camera : public OpticalElement
{
private:
    WaveFront sensed_wavefront;
    double size;

public:
    Camera(const vec3 &position, const vec3 &orientation, const std::string name, double size = 0.02); // Constructor
    virtual ~Camera() = default;                                                                       // Destructor

    double hit(const ray &beamlet) override;
    void interact_ray(ray &beamlet) override;
    void interact_wavefront(WaveFront &A) override;

    WaveFront &getSensedWaveFront();
    void reset();
};

#endif