#include "aperture.hpp"
#include "utils.hpp"
#include <cmath>
#include <iostream>

Iris::Iris(vec3 position, vec3 orientation, std::string name, double radius, double size)
    : OpticalElement(position, orientation, name), radius(min(radius, size)), size(size) {}

double Iris::hit(const ray &beamlet)
{
    double denom = dot(beamlet.dir(), getOrientation());
    if (std::abs(denom) < 1e-6)
        return -999.0;

    double t = dot(getPosition() - beamlet.pos(), getOrientation()) / denom;
    if (t < 1e-6)
        return -999.0;

    point3 hit_point = beamlet.pos() + beamlet.dir() * t;
    if ((hit_point - getPosition()).length_squared() > size * size)
        return -999.0;

    return t;
}

void Iris::interact_ray(ray &beamlet) {}

void Iris::interact_wavefront(WaveFront &A)
{
    double r_sq = radius * radius;
    vec3 displacement = A.getNormal().pos() - getPosition();
    double x_disp = dot(displacement, v);
    double y_disp = dot(displacement, u);

    for (int i = 0; i < A.N; i++)
    {
        double y = (A.N / 2 - i) * A.getPixelSize() + y_disp;
        for (int j = 0; j < A.N; j++)
        {
            double x = (A.N / 2 - j) * A.getPixelSize() + x_disp;

            if (x * x + y * y > r_sq)
            {
                A.Ex[i][j] *= 0.0;
                A.Ey[i][j] *= 0.0;
            }
        }
    }
}

Slit::Slit(vec3 position, vec3 orientation, std::string name, double size, double height, double width, int num_slits, double separation)
    : OpticalElement(position, orientation, name),
      size(size),
      height(min(size, height)),
      width(min(size, width)),
      separation(separation), // Initialize separation
      num_slits(num_slits)
{}

double Slit::hit(const ray &beamlet)
{
    double denom = dot(beamlet.dir(), getOrientation());
    if (std::abs(denom) < 1e-6) return -999.0;

    double t = dot(getPosition() - beamlet.pos(), getOrientation()) / denom;
    if (t < 1e-6) return -999.0;

    point3 hit_point = beamlet.pos() + beamlet.dir() * t;
    if ((hit_point - getPosition()).length_squared() > size * size)
        return -999.0;

    return t;
}

void Slit::interact_ray(ray &beamlet) {}

void Slit::interact_wavefront(WaveFront &A)
{
    std::vector<double> slit_centers;
    double start_x = -(num_slits - 1) * separation / 2.0;
    for(int k=0; k<num_slits; k++) {
        slit_centers.push_back(start_x + k * separation);
    }

    vec3 displacement = A.getNormal().pos() - getPosition();
    double x_disp = dot(displacement, v);
    double y_disp = dot(displacement, u);
    
    double half_width = width / 2.0;
    double half_height = height / 2.0;

    for (int i = 0; i < A.N; i++)
    {
        double x = (A.N / 2 - i) * A.getPixelSize() + x_disp;
        bool x_inside = false;
        for (double center_k : slit_centers) {
            if (std::abs(x - center_k) <= half_width) {
                x_inside = true;
                break;
            }
        }

        for (int j = 0; j < A.N; j++)
        {
            if (!x_inside) {
                A.Ex[j][i] *= 0.0;
                A.Ey[j][i] *= 0.0;
                continue;
            }

            double y = (A.N / 2 - j) * A.getPixelSize() + y_disp;
            
            if (std::abs(y) > half_height)
            {
                A.Ex[j][i] *= 0.0;
                A.Ey[j][i] *= 0.0;
            }
        }
    }
}