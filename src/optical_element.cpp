#include "optical_element.hpp"

// Constructor
OpticalElement::OpticalElement(const vec3 &pos, const vec3 &orient, const std::string &n)
    : position(pos), orientation(unit_vector(orient)), name(n)
{
    init_local_frame();
}

// Getters
vec3 OpticalElement::getPosition() const
{
    return position;
}

vec3 OpticalElement::getOrientation() const
{
    return orientation;
}

std::string OpticalElement::getName() const
{
    return name;
}

void OpticalElement::printDetails() const
{
    std::cout << "Name : " << getName() << "\tPosition : (" << position.x() << ", " << position.y() << ", " << position.z() << ")\tOrientation : (" << orientation.x() << ", " << orientation.y() << ", " << orientation.z() << ")" << std::endl;
}

void OpticalElement::setPosition(vec3 pos)
{
    position = pos;
}

void OpticalElement::setOrientation(vec3 o)
{
    orientation = o;
    init_local_frame();
}

void OpticalElement::init_local_frame()
{
    w = orientation;

    auto tmp = (abs(w.x()) < 0.9) ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);

    u = unit_vector(cross(w, tmp));
    v = unit_vector(cross(w, u));

    if (abs(w.x()) > 0.9)
    {
        tmp = u;
        u = v;
        v = tmp;
    }
}