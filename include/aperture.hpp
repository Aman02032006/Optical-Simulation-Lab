#ifndef APERTURE_HPP
#define APERTURE_HPP

#pragma once

#include "optical_element.hpp"

class Iris : public OpticalElement
{
private:
    double radius;
    double size;

public:
    Iris(vec3 position, vec3 orientation, std::string name, double radius, double size = 0.02);
    virtual ~Iris() = default;

    double getRadius() const { return radius; }
    double getSize() const { return size; }
    
    void setRadius(double r) { radius = min(size, r); }
    void setSize(double s) { size = s; }

    double hit(const ray &beamlet) override;
    void interact_ray(ray &beamlet) override;
    void interact_wavefront(WaveFront &A) override;
    void reset() override {};
};

class Slit : public OpticalElement
{
private:
    double size;
    double height;
    double width;
    int num_slits;
    double separation;

public:
    Slit(vec3 position, vec3 orientation, std::string name, double size = 0.02, double height = 0.01, double width = 1e-4, int num_slits = 1, double separation = 2e-4);
    virtual ~Slit() = default;

    double getSize() const { return size; }
    double getHeight() const { return height; }
    double getWidth() const { return width; }
    double getSeparation() const { return separation; }
    int getNumSlits() const { return num_slits; }
    
    void setSize(double s) { size = s; }
    void setHeight(double h) { height = min(size, h); }
    void setWidth(double w) { width = min(size, w); }
    void setSeparation(double s) { separation = s; }
    void setNumSlits(int n) { num_slits = n; }

    double hit(const ray &beamlet) override;
    void interact_ray(ray &beamlet) override;
    void interact_wavefront(WaveFront &A) override;
    void reset() override {};
};

#endif