#ifndef WAVEFRONT_H
#define WAVEFRONT_H

#pragma once

#include "ray.hpp" // for beam axis
#include <complex> // for Complex Electric Field amplitudes
#include <vector>  // for Grids
#include "fftw3.h" // for Fourier Transform

class WaveFront
{
private:
    double size;       // Size of the Wavefront Grid (For example, 5 X 5 cm)
    double pixel_size; // Size of individual pixels
    double wavelength; // Wavelength of the Wavefront
    ray normal;        // Normal to the wavefront plane
    FieldType source;  // Stores the type of source field
    double psi;        // Polarization angle
    double delta;      // Relative phase difference
    double w0;         // Beam specific parameters
    int l, p;

    inline int idx(int i, int j) const { return i * N + j; }

public:
    std::vector<std::vector<std::complex<double>>> Ex; // Grid of Amplitudes
    std::vector<std::vector<std::complex<double>>> Ey; // Grid of Polarizations
    vec3 u, v;                                         // Local frame for the wavefront plane
    int N;                                             // The Ex and Ey will be a N x N grid

    WaveFront(double size = 0.05, double pixel_size = 0.000005, ray normal, double wavelength, FieldType source, double psi, double delta, double w0, int l, int p);

    // Getters
    double getSize();
    double getPixelSize();
    double getWavelength();
    ray getNormal();

    void get_LocalFrame();                              // Sets up orthogonal vectors for the local plane of the wavefront
    void propagate(double z);                           // Propagates the wavefront a distance z using FFTW
    void phaseShift(double phi);                        // Applies a constant phase shift to the wavefront
    void scale(double factor);                          // Scales the wavefront
    void reflect(vec3 n);                               // Reflects the wavefront
    std::vector<std::vector<double>> Intensity() const; // returns an intensity map of the wavefront
    std::vector<std::vector<double>> Phase() const;     // returns a phase map of the wavefront

    void initialize(); // Initializes the Electric Field Grids according to the FieldType

    WaveFront operator+(const WaveFront &other);
    WaveFront operator-(const WaveFront &other);
    WaveFront &operator+=(const WaveFront &other);
    WaveFront &operator-=(const WaveFront &other);
};

#endif
