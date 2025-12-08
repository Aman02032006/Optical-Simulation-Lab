#include "camera.hpp"

Camera::Camera(const vec3 &position, const vec3 &orientation, std::string name, double size) : OpticalElement(position, orientation, name), size(size), sensed_wavefront(ray(position, orientation), 633e-9, FieldType::PLANE, 0.0, 0.0, 1e-3) {}

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

WaveFront &Camera::getSensedWaveFront()
{
    return sensed_wavefront;
}

void Camera::reset()
{
    sensed_wavefront.scale(0.0);
}

void Camera::setPosition(vec3 pos)
{
    OpticalElement::setPosition(pos);
    sensed_wavefront.setPosition(pos);
}

void Camera::setOrientation(vec3 o)
{
    OpticalElement::setOrientation(o);
    sensed_wavefront.setDirection(o);
}

void Camera::setSize(double s)
{
    size = s;
    sensed_wavefront.setSize(s);
}