#include "mirror.hpp"

Mirror::Mirror(const vec3 &position, const vec3 &orientation, const std::string name, double size, double reflectivity, std::complex<double> refractive_index)
    : OpticalElement(position, orientation, name),
      size(size),
      reflectivity(reflectivity),
      refractive_index(refractive_index) {}

double Mirror::hit(const ray &beamlet)
{
    double denom = dot(beamlet.dir(), getOrientation());
    if (fabs(denom) < 1e-6)
        return -999.0;

    double t = dot(getPosition() - beamlet.pos(), getOrientation()) / denom;

    if (t < 1e-6)
        return -999.0;

    point3 intersection_point = beamlet.pos() + beamlet.dir() * t;
    vec3 dist = intersection_point - getPosition();

    double x_local = dot(dist, v);
    double y_local = dot(dist, u);

    double half_size = size / 2.0;
    if (fabs(x_local) > half_size || fabs(y_local) > half_size)
        return -999.0;

    return t;
}

void Mirror::interact_ray(ray &beamlet)
{
    beamlet.reflect(getOrientation());
}

void Mirror::interact_wavefront(WaveFront &A)
{
    A.reflect(getOrientation());
}