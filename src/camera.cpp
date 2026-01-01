#include "camera.hpp"

Camera::Camera(const vec3 &position, const vec3 &orientation, std::string name, double size) : OpticalElement(position, orientation, name), size(size), sensedWavefront(ray(position, orientation), 633e-9, FieldType::BLANK, 0.0, 0.0, 1e-3, 0, 0, size, size / 1024.0)
{
    sensedWavefront.initialize();
}

double Camera::hit(const ray &beamlet)
{
    if (fabs(dot(beamlet.dir(), getOrientation())) < 1e-6)
        return -999.0;

    double t = -dot(beamlet.pos() - getPosition(), getOrientation()) / dot(beamlet.dir(), getOrientation());
    if (t < 0)
        return -999.0;

    point3 intersection = beamlet.pos() + beamlet.dir() * t;
    double x_point = dot(intersection - getPosition(), v);
    double y_point = dot(intersection - getPosition(), u);

    if (fabs(x_point) > size / 2 || fabs(y_point) > size / 2)
        return -999.0;

    return t;
}

void Camera::interact_ray(ray &beamlet)
{
    beamlet.kill();
}

void Camera::interact_wavefront(WaveFront &A)
{
    sensedWavefront += A;
    A.scale(0.0);
}

WaveFront &Camera::getSensedWaveFront()
{
    return  sensedWavefront;
}

void Camera::reset()
{
    sensedWavefront.initialize();
}

void Camera::setPosition(vec3 pos)
{
    OpticalElement::setPosition(pos);
    sensedWavefront.setPosition(pos);
}

void Camera::setOrientation(vec3 o)
{
    OpticalElement::setOrientation(o);
    sensedWavefront.setDirection(o);
}

void Camera::setSize(double s)
{
    size = s;
    sensedWavefront.setSize(s);
}