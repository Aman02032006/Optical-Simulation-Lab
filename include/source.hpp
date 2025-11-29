#ifndef SOURCE_H
#define SOURCE_H

#pragma once

#include "ray.hpp"
#include "optical_element.hpp"
#include "utils.hpp"
#include "vec3.hpp"
#include "wavefront.hpp"

class Source
{
private:
    vec3 position;
    vec3 orientation;
    FieldType mode;
    double psi;
    double delta;
    double wavelength;
    double w0;
    int l, p;

public:
    WaveFront E;
    Source(vec3 position, vec3 orientation, FieldType mode, double psi, double delta, double wavelength = 633e-9, double w0 = 1e-3, int l = 0, int p = 0)
        : position(position), orientation(orientation), mode(mode), psi(psi), delta(delta), wavelength(wavelength), w0(w0), l(l), p(p),
        E(ray(position, orientation), wavelength, mode, psi, delta, w0, l, p)
    {
        E.initialize();
    }

    vec3 getPosition() {
        return position ;
    }

    vec3 getOrientation() {
        return orientation ;
    }
};

#endif