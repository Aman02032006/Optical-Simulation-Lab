#ifndef LENS_HPP
#define LENS_HPP

#include "optical_element.hpp"

class ConvexLens : public OpticalElement {
private:
    double radius;
    double focalLength;
    double n;

public:
    ConvexLens(vec3 position, vec3 orientation, std::string name, double diameter, double focal_length, double refractive_index);

    double hit(const ray &beamlet) override;
    void interact_ray(ray &beamlet) override;
    void interact_wavefront(WaveFront &A) override;
    void reset() override {};

    void setRadius(double r) { radius = r; };
    void setFocalLength(double f) { focalLength = f; };
    void setRefractiveIndex(double refr_index) { n = refr_index; };

    double getRadius() const { return radius; };
    double getFocalLength() const { return focalLength; };
    double getRefractiveIndex() const { return n; };
};

class ConcaveLens : public OpticalElement {
private:
    double radius;
    double focalLength;
    double n;

public:
    ConcaveLens(vec3 position, vec3 orientation, std::string name, double diameter, double focalLength, double refractive_index);

    double hit(const ray &beamlet) override;
    void interact_ray(ray &beamlet) override;
    void interact_wavefront(WaveFront &A) override;
    void reset() override {};

    void setRadius(double r) { radius = r; };
    void setFocalLength(double f) { focalLength = f; };
    void setRefractiveIndex(double refr_index) { n = refr_index; };

    double getRadius() const { return radius; };
    double getFocalLength() const { return focalLength; };
    double getRefractiveIndex() const { return n; };
};

#endif