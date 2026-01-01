#ifndef CAMERA_HPP
#define CAMERA_HPP

#pragma once

#include "optical_element.hpp"

class Camera : public OpticalElement
{
private:
    WaveFront sensedWavefront;
    double size;

public:
    Camera(const vec3 &position, const vec3 &orientation, const std::string name, double size = 0.02); // Constructor
    virtual ~Camera() = default;                                                                       // Destructor

    double getSize() { return size; }
    WaveFront &getSensedWaveFront();

    void setPosition(vec3 pos) override;
    void setOrientation(vec3 o) override;
    void setSize(double s);

    double hit(const ray &beamlet) override;
    void interact_ray(ray &beamlet) override;
    void interact_wavefront(WaveFront &A) override;
    void reset() override;
};

#endif