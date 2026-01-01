#ifndef MIRROR_HPP
#define MIRROR_HPP

#pragma once

#include "optical_element.hpp"
#include "utils.hpp"

class Mirror : public OpticalElement
{
private:
    double size;
    double reflectivity;
    std::complex<double> refractive_index;

public:
    Mirror(const vec3 &position, const vec3 &orientation, const std::string name, double size = 0.02, double reflectivity = 1.0, std::complex<double> refractive_index = {1.5, 0});
    virtual ~Mirror() = default;

    double getSize() const { return size; }
    double getReflectivity() const { return reflectivity; }
    std::complex<double> getRefractiveIndex() const { return refractive_index; }

    void setSize(double new_size) { size = new_size; }
    void setReflectivity(double new_reflectivity) { reflectivity = new_reflectivity; }
    void setRefractiveIndex(std::complex<double> new_RI) { refractive_index = new_RI; }

    double hit(const ray &beamlet) override;
    void interact_ray(ray &beamlet) override;
    void interact_wavefront(WaveFront &A) override;
    void reset() override {}
};

#endif