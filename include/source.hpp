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
          E(ray(position, orientation), wavelength, mode, psi, delta, w0, l, p) {}

    vec3 getPosition() { return position; }
    vec3 getOrientation() { return orientation; }
    FieldType getFieldType() { return mode; }
    double getPsi() { return psi; }
    double getDelta() { return delta; }
    double getWavelength() { return wavelength; }
    double getBeamWaist() { return w0; }
    int getL() { return l; }
    int getP() { return p; }

    void setPosition(vec3 pos)
    {
        position = pos;
        E.setPosition(pos);
    }

    void setOrientation(vec3 o)
    {
        orientation = o;
        E.setDirection(o);
    }

    void setFieldType(FieldType type)
    {
        mode = type;
        E.setFieldType(type);
    }

    void setPsi(double theta)
    {
        psi = theta;
        E.setPsi(theta);
    }

    void setDelta(double theta)
    {
        delta = theta;
        E.setDelta(theta);
    }

    void setWavelength(double w)
    {
        wavelength = w;
        E.setWavelength(w);
    }

    void setBeamWaist(double w)
    {
        w0 = w;
        E.setBeamWaist(w);
    }

    void setBeamMode(int L, int P)
    {
        l = L;
        p = P;
        E.setBeamMode(L, P);
    }
};

#endif