#include "WaveFront.hpp"
#include "utils.hpp"
#include <stdexcept>
#include <cmath>

// Constructor
WaveFront::WaveFront(ray normal, double wavelength, FieldType source, double psi, double delta, double w0, int l, int p, double size, double pixel_size)
    : size(size), pixel_size(pixel_size), normal(normal), wavelength(wavelength), source(source), w0(w0), l(l), p(p), psi(psi), delta(delta)
{
    N = (int)(size / pixel_size);
    Ex.resize(N, std::vector<std::complex<double>>(N, {0.0, 0.0}));
    Ey.resize(N, std::vector<std::complex<double>>(N, {0.0, 0.0}));
    get_LocalFrame();
}

// Getters
double WaveFront::getSize() { return size; }
double WaveFront::getPixelSize() { return pixel_size; }
double WaveFront::getWavelength() { return wavelength; }
ray WaveFront::getNormal() { return normal; }

// Local frame computation
void WaveFront::get_LocalFrame()
{
    w = normal.dir();
    vec3 temp = (std::abs(w.x()) < 0.9) ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);

    u = unit_vector(cross(w, temp));
    v = unit_vector(cross(w, u));

    if (w.x() > 0.9)
    {
        temp = u;
        u = v;
        v = temp;
    }
}

// Fresnel propagation
void WaveFront::propagate(double z)
{
    if (z == 0.0)
        return;

    normal.propagate(z);

    const double dx = pixel_size;
    const double k_mag = 2 * PI / wavelength;

    fftw_complex *inp = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N * N);
    fftw_complex *out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N * N);

    if (!inp || !out)
    {
        if (inp)
            fftw_free(inp);
        if (out)
            fftw_free(out);
        throw std::bad_alloc();
    }

    auto process_component = [&](const std::vector<std::vector<std::complex<double>>> &A,
                                 std::vector<std::vector<std::complex<double>>> &Aout)
    {
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
            {
                int kidx = idx(i, j);
                inp[kidx][0] = A[i][j].real();
                inp[kidx][1] = A[i][j].imag();
            }

        fftw_plan forward = fftw_plan_dft_2d(N, N, inp, out, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(forward);
        fftw_destroy_plan(forward);

        for (int u = 0; u < N; ++u)
        {
            int uu = u - N / 2;
            double fx = double(uu) / (N * dx);
            for (int v = 0; v < N; ++v)
            {
                int vv = v - N / 2;
                double fy = double(vv) / (N * dx);
                double H_phase = -PI * wavelength * z * (fx * fx + fy * fy);
                std::complex<double> H = std::polar(1.0, H_phase);

                int kidx = idx(u, v);
                std::complex<double> S(out[kidx][0], out[kidx][1]);
                S *= H;

                out[kidx][0] = S.real();
                out[kidx][1] = S.imag();
            }
        }

        fftw_plan inverse = fftw_plan_dft_2d(N, N, out, inp, FFTW_BACKWARD, FFTW_ESTIMATE);
        fftw_execute(inverse);
        fftw_destroy_plan(inverse);

        double norm = 1.0 / double(N * N);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
            {
                int kidx = idx(i, j);
                std::complex<double> val(inp[kidx][0] * norm, inp[kidx][1] * norm);
                Aout[i][j] = val;
            }
    };

    auto apply_shift = [&](std::vector<std::vector<std::complex<double>>> &A)
    {
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                A[i][j] *= std::complex<double>((((i + j) & 1) ? -1.0 : 1.0), 0.0);
    };

    std::vector<std::vector<std::complex<double>>> Ex_out(N, std::vector<std::complex<double>>(N));
    std::vector<std::vector<std::complex<double>>> Ey_out(N, std::vector<std::complex<double>>(N));

    auto Ex_copy = Ex;
    apply_shift(Ex_copy);
    process_component(Ex_copy, Ex_out);
    apply_shift(Ex_out);

    auto Ey_copy = Ey;
    apply_shift(Ey_copy);
    process_component(Ey_copy, Ey_out);
    apply_shift(Ey_out);

    Ex.swap(Ex_out);
    Ey.swap(Ey_out);

    fftw_free(inp);
    fftw_free(out);
}

// Phase shift
void WaveFront::phaseShift(double phi)
{
    std::complex<double> ph = std::polar(1.0, phi);
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
        {
            Ex[i][j] *= ph;
            Ey[i][j] *= ph;
        }
}

// Scaling
void WaveFront::scale(double factor)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
        {
            Ex[i][j] *= factor;
            Ey[i][j] *= factor;
        }
}

// Intensity
std::vector<std::vector<double>> WaveFront::Intensity() const
{
    std::vector<std::vector<double>> intensity(N, std::vector<double>(N, 0.0));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            intensity[i][j] = sq(std::norm(Ex[i][j])) + sq(std::norm(Ey[i][j]));
    return intensity;
}

std::vector<std::vector<double>> WaveFront::Phase() const
{
    std::vector<std::vector<double>> phase(N, std::vector<double>(N, 0.0));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            phase[i][j] = std::arg(Ex[i][j]);
    return phase;
}

void WaveFront::initialize()
{
    double k = 2 * PI / wavelength;
    double half = size / 2.0;

    for (int i = 0; i < N; i++)
    {
        double x = (i - N / 2) * pixel_size;
        for (int j = 0; j < N; j++)
        {
            double y = (j - N / 2) * pixel_size;
            std::complex<double> comp_amp(0.0, 0.0);
            double norm = sqrt(2.0 / (PI * w0 * w0));

            switch (source)
            {
            case FieldType::PLANE:
                comp_amp = std::polar(1.0, 0.0);
                break;

            case FieldType::GAUSSIAN:
            {
                double r2 = x * x + y * y;
                double amp = norm * exp(-r2 / (w0 * w0));
                comp_amp = std::complex<double>(amp, 0.0);
                break;
            }

            case FieldType::LG:
            {
                double r = sqrt(x * x + y * y);
                double phi = atan2(y, x);
                double rho = sqrt(2.0) * r / w0;
                double L = genLaguerre(p, std::abs(l), rho * rho);
                double amp = L * pow(rho, std::abs(l)) * exp(-rho * rho / 2.0);

                double norm = sqrt(2.0 * factorial(p) / (PI * factorial(p + std::abs(l)))) / w0;

                amp *= norm;

                double phase = l * phi + k * dot(normal.dir(), vec3(x, y, 0.0));

                comp_amp = std::polar(amp, phase);
                break;
            }

            case FieldType::HG:
            {
                double X = sqrt(2.0) * x / w0;
                double Y = sqrt(2.0) * y / w0;
                double Hm = hermitePol(l, X);
                double Hn = hermitePol(p, Y);
                comp_amp = std::complex<double>(norm * Hm * Hn * exp(-(x * x + y * y) / (w0 * w0)), 0.0);
                break;
            }
            }

            Ex[i][j] = comp_amp * std::cos(psi);
            Ey[i][j] = comp_amp * std::polar(1.0, delta) * std::sin(psi);
        }
    }
}

// Operators
WaveFront WaveFront::operator+(const WaveFront &other)
{
    WaveFront C(this->getNormal(), this->getWavelength(), this->source, this->psi, this->delta, this->w0, this->l, this->p, this->getSize(), this->getPixelSize());
    for (int i = 0; i < C.N; i++)
        for (int j = 0; j < C.N; j++)
        {
            C.Ex[i][j] = this->Ex[i][j] + other.Ex[i][j];
            C.Ey[i][j] = this->Ey[i][j] + other.Ey[i][j];
        }
    return C;
}

WaveFront WaveFront::operator-(const WaveFront &other)
{
    WaveFront C(this->getNormal(), this->getWavelength(), this->source, this->psi, this->delta, this->w0, this->l, this->p, this->getSize(), this->getPixelSize());
    for (int i = 0; i < C.N; i++)
        for (int j = 0; j < C.N; j++)
        {
            C.Ex[i][j] = this->Ex[i][j] - other.Ex[i][j];
            C.Ey[i][j] = this->Ey[i][j] - other.Ey[i][j];
        }
    return C;
}

WaveFront &WaveFront::operator+=(const WaveFront &other)
{
    const double k = 2.0 * PI / other.wavelength;

    std::cout << "[Wavefront] : " << k << std::endl;
    std::cout << "[Wavefront] : " << other.w << std::endl;
    std::cout << "[Wavefront] : " << this->w << std::endl;

    vec3 this_center = this->normal.pos();
    vec3 this_normal = this->w;
    vec3 other_center = other.normal.pos();
    vec3 other_normal = other.w;

    double denominator = dot(other_normal, this_normal);

    for (int i = 0; i < other.N; i++)
    {
        for (int j = 0; j < other.N; j++)
        {
            vec3 SourcePixelPosition = other_center + (i - other.N / 2.0) * other.pixel_size * other.u + (j - other.N / 2.0) * other.pixel_size * other.v;
            double DistanceToThisPlane = 0.0;
            if (std::abs(denominator) >= 1e-6)
                DistanceToThisPlane = dot(this_center - SourcePixelPosition, this_normal) / denominator;
            SourcePixelPosition += DistanceToThisPlane * other_normal;

            int iThis = (int)(dot(SourcePixelPosition - this_center, this->u) / this->pixel_size + this->N / 2.0);
            int jThis = (int)(dot(SourcePixelPosition - this_center, this->v) / this->pixel_size + this->N / 2.0);

            if (iThis >= 0 && iThis < this->N && jThis >= 0 && jThis < this->N)
            {
                std::complex<double> PhaseFactor = std::polar(1.0, k * DistanceToThisPlane);
                this->Ex[iThis][jThis] += other.Ex[i][j] * PhaseFactor;
                this->Ey[iThis][jThis] += other.Ey[i][j] * PhaseFactor;
            }
        }
    }

    std::cout << "[Wavefront] : Wavefront added" << std::endl;

    return *this;
}

WaveFront &WaveFront::operator-=(const WaveFront &other)
{
    for (int i = 0; i < this->N; i++)
        for (int j = 0; j < this->N; j++)
        {
            this->Ex[i][j] -= other.Ex[i][j];
            this->Ey[i][j] -= other.Ey[i][j];
        }
    return *this;
}

// Reflection
void WaveFront::reflect(vec3 n)
{
    normal.reflect(n);
    get_LocalFrame();
}

void WaveFront::setPosition(vec3 pos)
{
    normal = ray(pos, normal.dir());
}

void WaveFront::setDirection(vec3 dir)
{
    normal = ray(normal.pos(), dir);
    get_LocalFrame();
}

void WaveFront::setSize(double s)
{
    size = s;
}

void WaveFront::setPixelSize(double px)
{
    pixel_size = px;
}

void WaveFront::setWavelength(double w)
{
    wavelength = w;
}

void WaveFront::setFieldType(FieldType type)
{
    source = type;
}

void WaveFront::setPsi(double theta)
{
    psi = theta;
}

void WaveFront::setDelta(double theta)
{
    delta = theta;
}

void WaveFront::setBeamWaist(double w)
{
    w0 = w;
}

void WaveFront::setBeamMode(int L, int P)
{
    l = L;
    p = P;
}
