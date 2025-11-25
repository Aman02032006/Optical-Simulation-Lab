#include "camera.hpp"

Camera::Camera(const vec3 &position, const vec3 &orientation, std::string name, double size) : OpticalElement(position, orientation, name), size(size) {}

double Camera::hit(const ray &beamlet)
{
    if (abs(dot(beamlet.dir(), getOrientation())) < 1e-6)
        return -999.0;

    double t = -dot(beamlet.pos() - getPosition(), getOrientation()) / dot(beamlet.dir(), getOrientation());
    if (t < 0)
        return -999.0;

    point3 intersection = beamlet.pos() + beamlet.dir() * t;
    double x_point = dot(intersection - getPosition(), v);
    double y_point = dot(intersection - getPosition(), u);

    if (abs(x_point) > size / 2 || abs(y_point) > size / 2)
        return -999.0;

    return t;
}

void Camera::interact_ray(ray &beamlet)
{
    beamlet.kill();
}

void Camera::interact_wavefront(WaveFront &A)
{
    sensed_wavefront += A;
    A.scale(0.0);
}

void Camera::output()
{
    using namespace matplot;

    auto intensity = sensed_wavefront.Intensity();
    auto phase = sensed_wavefront.Phase();

    auto fig = figure(true);
    fig->name(getName() + " Output");

    auto ax1 = subplot(1, 2, 0);
    imagesc(ax1, intensity);
    title(ax1, "Intensity Map");
    colorbar(ax1);
    axis(ax1, equal);

    auto ax2 = subplot(1, 2, 1);
    imagesc(ax2, phase);
    title(ax2, "Phase Map");
    colorbar(ax2);
    axis(ax2, equal);

    show();
}