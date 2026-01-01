#include "lens.hpp"
#include "utils.hpp"

ConvexLens::ConvexLens(vec3 position, vec3 orientation, std::string name, double diameter, double focalLength, double refractive_index)
    : OpticalElement(position, orientation, name), radius(diameter / 2.0), focalLength(focalLength), n(refractive_index) {}

double ConvexLens::hit(const ray &beamlet)
{
    double denom = dot(beamlet.dir(), getOrientation());
    if (std::abs(denom) < 1e-6)
        return -999.0;

    double t = dot(getPosition() - beamlet.pos(), getOrientation()) / denom;
    if (t < 1e-6)
        return -999.0; // Behind the ray origin

    point3 hit_point = beamlet.pos() + beamlet.dir() * t;
    if ((hit_point - getPosition()).length_squared() > radius * radius)
        return -999.0; // Aperture check

    return t;
}

void ConvexLens::interact_ray(ray &beamlet)
{

    vec3 r_vec = beamlet.pos() - getPosition();
    vec3 direction_change = -(r_vec / focalLength);

    vec3 new_dir = unit_vector(beamlet.dir() + direction_change);
    beamlet.setDirection(new_dir);
}

void ConvexLens::interact_wavefront(WaveFront &A)
{
    double k = 2 * PI / A.getWavelength();
    double prefactor = -k / (2.0 * focalLength);

    for (int i = 0; i < A.N; i++)
    {
        double x = (i - A.N / 2) * A.getPixelSize();
        for (int j = 0; j < A.N; j++)
        {
            double y = (j - A.N / 2) * A.getPixelSize();
            double r2 = x * x + y * y;

            if (r2 <= radius * radius)
            {
                std::complex<double> lens_phasor = std::polar(1.0, prefactor * r2);
                A.Ex[i][j] *= lens_phasor;
                A.Ey[i][j] *= lens_phasor;
            }
        }
    }
}

ConcaveLens::ConcaveLens(vec3 position, vec3 orientation, std::string name, double diameter, double focalLength, double refractive_index)
    : OpticalElement(position, orientation, name), radius(diameter / 2.0), focalLength(focalLength), n(refractive_index) {}

double ConcaveLens::hit(const ray &beamlet)
{
    double denom = dot(beamlet.dir(), getOrientation());
    if (std::abs(denom) < 1e-6)
        return -999.0;

    double t = dot(getPosition() - beamlet.pos(), getOrientation()) / denom;
    if (t < 1e-6)
        return -999.0;

    point3 hit_point = beamlet.pos() + beamlet.dir() * t;
    if ((hit_point - getPosition()).length_squared() > radius * radius)
        return -999.0;

    return t;
}

void ConcaveLens::interact_ray(ray &beamlet)
{

    vec3 r_vec = beamlet.pos() - getPosition();
    vec3 direction_change = (r_vec / focalLength);

    vec3 new_dir = unit_vector(beamlet.dir() + direction_change);
    beamlet.setDirection(new_dir);
}

void ConcaveLens::interact_wavefront(WaveFront &A)
{
    double k = 2 * PI / A.getWavelength();
    double prefactor = k / (2.0 * focalLength);

    for (int i = 0; i < A.N; i++)
    {
        double x = (i - A.N / 2) * A.getPixelSize();
        for (int j = 0; j < A.N; j++)
        {
            double y = (j - A.N / 2) * A.getPixelSize();
            double r2 = x * x + y * y;

            if (r2 <= radius * radius)
            {
                std::complex<double> lens_phasor = std::polar(1.0, prefactor * r2);
                A.Ex[i][j] *= lens_phasor;
                A.Ey[i][j] *= lens_phasor;
            }
            else {
                A.Ex[i][j] *= 0;
                A.Ey[i][j] *= 0;
            }
        }
    }
}