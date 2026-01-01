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
    v = vec3(w.z(), 0.0, -w.x());
    v = unit_vector(v);
    u = cross(w, v);
}