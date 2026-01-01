#include "utils.hpp"
#include <cmath>

double genLaguerre(int p, int l, double x)
{
    double sum = 0.0;
    for (int m = 0; m <= p; ++m)
    {
        double term = pow(-1, m) * factorial(p + l) / (factorial(p - m) * factorial(l + m) * factorial(m)) * pow(x, m);
        sum += term;
    }
    return sum;
}

double hermitePol(int n, double x)
{
    if (n == 0)
        return 1.0;
    if (n == 1)
        return 2.0 * x;

    double Hnm2 = 1.0, Hnm1 = 2.0 * x, Hn = 0.0;

    for (int i = 2; i <= n; ++i)
    {
        Hn = 2.0 * x * Hnm1 - 2.0 * (i - 1) * Hnm2;
        Hnm2 = Hnm1;
        Hnm1 = Hn;
    }
    return Hn;
}