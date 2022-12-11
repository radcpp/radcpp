#include "rad/Core/Math.h"

// Finds solutions of the quadratic equation at^2 + bt + c = 0; return true if solutions were found.
// Please refer to: https://pbr-book.org/3ed-2018/Utilities/Mathematical_Routines
bool SolveQuadraticEquation(float a, float b, float c, float& t0, float& t1)
{
    // b^2 - 4ac
    double discrim = (double)b * (double)b - 4 * (double)a * (double)c;
    if (discrim < 0) return false;
    double rootDiscrim = std::sqrt(discrim);
    double q = 0.0;
    if (b < 0)
    {
        q = -.5 * (b - rootDiscrim);
    }
    else
    {
        q = -.5 * (b + rootDiscrim);
    }
    t0 = static_cast<float>(q / a);
    t1 = static_cast<float>(c / q);
    if (t0 > t1)
    {
        std::swap(t0, t1);
    }
    return true;
}
