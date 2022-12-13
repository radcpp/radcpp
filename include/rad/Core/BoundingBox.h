#pragma once

#include "rad/Core/Common.h"
#include "rad/Core/Geometry.h"

// Bounds2: https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/vecmath.h
template<typename T>
struct BoundingBox2D
{
    union
    {
        Point2D<T> m_bounds[2];
        struct Bounds
        {
            Point2D<T> min;
            Point2D<T> max;
        } m;
    };

    BoundingBox2D()
    {
        SetEmpty();
    }

    explicit BoundingBox2D(const Point2D<T>& p)
    {
        m.min = p;
        m.max = p;
    }

    BoundingBox2D(const Point2D<T>& min, const Point2D<T>& max)
    {
        m.min = min;
        m.max = max;
    }

    template<typename U>
    BoundingBox2D(const BoundingBox2D<U>& b)
    {
        if (!b.IsEmpty())
        {
            m.min = Point2D<T>(b.m.min);
            m.max = Point2D<T>(b.m.max);
        }
        else
        {
            SetEmpty();
        }
    }

    void SetEmpty()
    {
        T minValue = std::numeric_limits<T>::lowest();
        T maxValue = std::numeric_limits<T>::max();
        m.min = Point2D<T>(maxValue, maxValue);
        m.max = Point2D<T>(minValue, minValue);
    }

    bool IsEmpty() const
    {
        return (m.min.x >= m.max.x) || (m.min.y >= m.max.y);
    }

    bool IsDegenerate() const
    {
        return (m.min.x > m.max.x) || (m.min.y > m.max.y);
    }

    Vector2D<T> GetDiagnal() const
    {
        return (m.max - m.min);
    }

    T GetArea() const
    {
        Vector2D<T> d = GetDiagnal();
        return (d.x * d.y);
    }

    int GetMaxDimension() const
    {
        Vector2D<T> d = GetDiagnal();
        if (d.x > d.y)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }

    const Point2D<T>& operator[](size_t i) const
    {
        assert(i == 0 || i == 1);
        return m_bounds[i];
    }

    Point2D<T>& operator[](size_t i)
    {
        assert(i == 0 || i == 1);
        return m_bounds[i];
    }

    bool operator==(const BoundingBox2D<T>& b) const
    {
        return (m.min == b.m.min) && (m.max == b.m.max);
    }

    bool operator!=(const BoundingBox2D<T>& b) const
    {
        return (m.min != b.m.min) || (m.max != b.m.max);
    }

    Point2D<T> GetCorner(size_t i) const
    {
        assert(i >= 0 && i < 4);
        return Point2D<T>(m_bounds[i & 1].x, m_bounds[(i & 2) ? 1 : 0].y);
    }

    template<typename Float>
    Point2D<Float> Lerp(const Point2D<Float>& t) const
    {
        return Point2D<Float>(
            glm::lerp(t.x, Float(m.min.x), Float(m.max.x)),
            glm::lerp(t.y, Float(m.min.y), Float(m.max.y))
            );
    }

    template<typename Float>
    Vector2D<Float> GetNormalizedOffset(const Point2D<Float>& p)
    {
        assert(!IsEmpty());
        Point2D<Float> offset = Point2D<Float>(p) - Point2D<Float>(m.min);
        offset.x /= Float(m.max.x) - Float(m.min.x);
        offset.y /= Float(m.max.y) - Float(m.min.y);
        return offset;
    }

}; // class BoundingBox2D

// Bounds3: https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/vecmath.h
template<typename T>
struct BoundingBox3D
{
    union
    {
        Point3D<T> m_bounds[2];
        struct Bounds
        {
            Point3D<T> min;
            Point3D<T> max;
        } m;
    };

    BoundingBox3D()
    {
        SetEmpty();
    }

    explicit BoundingBox3D(const Point3D<T>& p)
    {
        m.min = p;
        m.max = p;
    }

    BoundingBox3D(const Point3D<T>& min, const Point3D<T>& max)
    {
        m.min = min;
        m.max = max;
    }

    template<typename U>
    BoundingBox3D(const BoundingBox3D<U>& b)
    {
        if (!b.IsEmpty())
        {
            m.min = Point3D<T>(b.m.min);
            m.max = Point3D<T>(b.m.max);
        }
        else
        {
            SetEmpty();
        }
    }

    void SetEmpty()
    {
        T minValue = std::numeric_limits<T>::lowest();
        T maxValue = std::numeric_limits<T>::max();
        m.min = Point3D<T>(maxValue, maxValue, maxValue);
        m.max = Point3D<T>(minValue, minValue, minValue);
    }

    bool IsEmpty() const
    {
        return (m.min.x >= m.max.x) || (m.min.y >= m.max.y) || (m.min.z >= m.max.z);
    }

    bool IsDegenerate() const
    {
        return (m.min.x > m.max.x) || (m.min.y > m.max.y) || (m.min.z > m.max.z);
    }

    const Point3D<T>& operator[](size_t i) const
    {
        assert(i == 0 || i == 1);
        return m_bounds[i];
    }

    Point3D<T>& operator[](size_t i)
    {
        assert(i == 0 || i == 1);
        return m_bounds[i];
    }

    Point3D<T> GetCorner(size_t i) const
    {
        assert(i >= 0 && i < 8);
        return Point3D<T>(
            m_bounds[(i & 1)].x,
            m_bounds[(i & 2) ? 1 : 0].y,
            m_bounds[(i & 4) ? 1 : 0].z);
    }

    Vector3D<T> GetDiagonal() const
    {
        return (m.max - m.min);
    }

    T GetSurfaceArea() const
    {
        Vector3D<T> d = GetDiagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

    T GetVolume() const
    {
        Vector3D<T> d = GetDiagonal();
        return d.x * d.y * d.z;
    }

    // 0-x; 1-y; 2-z
    int GetMaxDimension() const
    {
        Vector3D<T> d = GetDiagonal();
        if (d.x > d.y && d.x > d.z)
        {
            return 0;
        }
        else if (d.y > d.z)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }

    template<typename Float>
    Point3D<Float> Lerp(const Point3D<Float>& t)
    {
        return Point3D<Float>(
            glm::lerp(t.x, Float(m.min.x), Float(m.max.x)),
            glm::lerp(t.y, Float(m.min.y), Float(m.max.y)),
            glm::lerp(t.z, Float(m.min.z), Float(m.max.z))
            );
    }

    template<typename Float>
    Vector3D<Float> GetNormalizedOffset(const Point3D<Float>& p)
    {
        assert(!IsEmpty());
        Vector3D<Float> offset = Vector3D<Float>(p) - Vector3D<Float>(m.min);
        offset.x /= Float(m.max.x) - Float(m.min.x);
        offset.y /= Float(m.max.y) - Float(m.min.y);
        offset.z /= Float(m.max.z) - Float(m.min.z);
        return offset;
    }

    bool operator==(const BoundingBox2D<T>& b) const
    {
        return (m.min == b.m.min) && (m.max == b.m.max);
    }

    bool operator!=(const BoundingBox2D<T>& b) const
    {
        return (m.min != b.m.min) || (m.max != b.m.max);
    }

}; // class BoundingBox3D

template<typename T>
inline BoundingBox2D<T> Union(const BoundingBox2D<T>& b1, const BoundingBox2D<T>& b2)
{
    assert(!b1.IsEmpty() && !b2.IsEmpty());
    BoundingBox2D<T> b;
    b.m.min = glm::min(b1.m.min, b2.m.min);
    b.m.max = glm::max(b1.m.max, b2.m.max);
    return b;
}

template<typename T>
inline BoundingBox2D<T> Intersect(const BoundingBox2D<T>& b1, const BoundingBox2D<T>& b2)
{
    assert(!b1.IsEmpty() && !b2.IsEmpty());
    BoundingBox2D<T> b;
    b.m.min = glm::max(b1.m.min, b2.m.min);
    b.m.max = glm::min(b1.m.max, b2.m.max);
    return b;
}

template<typename T>
inline bool HasOverlap(const BoundingBox2D<T>& b1, const BoundingBox2D<T>& b2)
{
    assert(!b1.IsEmpty() && !b2.IsEmpty());
    bool x = (b1.m.max.x >= b2.m.min.x) && (b1.m.min.x <= b2.m.max.x);
    bool y = (b1.m.max.y >= b2.m.min.y) && (b1.m.min.y <= b2.m.max.y);
    return (x && y);
}

template<typename T>
inline bool IsInside(const Point2D<T>& p, const BoundingBox2D<T>& b)
{
    assert(!b.IsEmpty());
    return(p.x >= b.m.min.x && p.x <= b.m.max.x &&
        p.y >= b.m.min.y && p.y <= b.m.max.y);
}

template<typename T>
inline bool IsInsideExclusive(const Point2D<T>& p, const BoundingBox2D<T>& b)
{
    assert(!b.IsEmpty());
    return(p.x > b.m.min.x && p.x < b.m.max.x&&
        p.y > b.m.min.y && p.y < b.m.max.y);
}

template<typename T, typename U>
inline BoundingBox2D<T> Expand(const BoundingBox2D<T>& b, U delta)
{
    assert(!b.IsEmpty());
    BoundingBox2D<T> ret;
    ret.m.min = b.m.min - Vector2D<T>(delta, delta);
    ret.m.max = b.m.max + Vector2D<T>(delta, delta);
    return ret;
}

template<typename T>
inline BoundingBox3D<T> Union(const BoundingBox3D<T>& b1, const BoundingBox3D<T>& b2)
{
    assert(!b1.IsEmpty() && !b2.IsEmpty());
    BoundingBox3D<T> b;
    b.m.min = glm::min(b1.m.min, b2.m.min);
    b.m.max = glm::max(b1.m.max, b2.m.max);
    return b;
}

template<typename T>
inline BoundingBox3D<T> Union(const BoundingBox3D<T>& b, const Point3D<T>& p)
{
    assert(!b.IsEmpty());
    BoundingBox3D<T> ret;
    ret.m.min = glm::min(b.m.min, p);
    ret.m.max = glm::max(b.m.max, p);
    return ret;
}

template<typename T>
inline BoundingBox3D<T> Intersect(const BoundingBox3D<T>& b1, const BoundingBox3D<T>& b2)
{
    assert(!b1.IsEmpty() && !b2.IsEmpty());
    BoundingBox3D<T> b;
    b.m.min = glm::max(b1.m.min, b2.m.min);
    b.m.max = glm::min(b1.m.max, b2.m.max);
    return b;
}

template<typename T>
inline bool HasOverlap(const BoundingBox3D<T>& b1, const BoundingBox3D<T>& b2)
{
    assert(!b1.IsEmpty() && !b2.IsEmpty());
    bool x = (b1.m.max.x >= b2.m.min.x) && (b1.m.min.x <= b2.m.max.x);
    bool y = (b1.m.max.y >= b2.m.min.y) && (b1.m.min.y <= b2.m.max.y);
    bool z = (b1.m.max.z >= b2.m.min.z) && (b1.m.min.z <= b2.m.max.z);
    return (x && y && z);
}

template<typename T>
inline bool IsInside(const Point3D<T>& p, const BoundingBox3D<T>& b)
{
    assert(!b.IsEmpty());
    return(p.x >= b.m.min.x && p.x <= b.m.max.x &&
        p.y >= b.m.min.y && p.y <= b.m.max.y &&
        p.z >= b.m.min.z && p.z <= b.m.max.z);
}

template<typename T>
inline bool IsInsideExclusive(const Point3D<T>& p, const BoundingBox3D<T>& b)
{
    assert(!b.IsEmpty());
    return(p.x > b.m.min.x && p.x < b.m.max.x&&
        p.y > b.m.min.y && p.y < b.m.max.y&&
        p.z > b.m.min.z && p.z < b.m.max.z);
}

template<typename T, typename U>
inline BoundingBox3D<T> Expand(const BoundingBox3D<T>& b, U delta)
{
    assert(!b.IsEmpty());
    BoundingBox3D<T> ret;
    ret.m.min = b.m.min - Vector3D<T>(delta, delta, delta);
    ret.m.max = b.m.max + Vector3D<T>(delta, delta, delta);
    return ret;
}

// https://www.pbr-book.org/3ed-2018/Shapes/Managing_Rounding_Error#x1-ErrorPropagation
template<typename Float>
inline constexpr Float gamma(int n)
{
    const Float MachineEpsilon = std::numeric_limits<Float>::epsilon() * 0.5f;
    return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
}

// https://www.pbr-book.org/3ed-2018/Shapes/Basic_Shape_Interface#RayndashBoundsIntersections
template <typename T, typename Float>
inline bool IntersectRay(const BoundingBox3D<T>& bbox, const Point3D<Float>& origin, const Vector3D<Float>& invDir, Float tMax,
    Float& t0, Float& t1)
{
    t0 = Float(0);
    t1 = tMax;
    for (int i = 0; i < 3; ++i)
    {
        // Update interval for _i_th bounding box slab
        Float tNear = (bbox.m.min[i] - origin[i]) * invDir[i];
        Float tFar = (bbox.m.max[i] - origin[i]) * invDir[i];
        // Update parametric interval from slab intersection $t$ values
        if (tNear > tFar)
        {
            std::swap(tNear, tFar);
        }
        // Update _tFar_ to ensure robust ray--bbox intersection
        tFar *= 1 + 2 * gamma<Float>(3);

        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar < t1 ? tFar : t1;
        if (t0 > t1)
        {
            return false;
        }
    }
    return true;
}

// https://www.pbr-book.org/3ed-2018/Shapes/Basic_Shape_Interface#RayndashBoundsIntersections
// @param dirSigns: precomputed ray direction signs: 0 for positive and 1 for negative.
template <typename T, typename Float>
inline bool IntersectRay(const BoundingBox3D<T>& bbox, const Point3D<Float>& origin, const Vector3D<Float>& invDir,
    const Vector3D<int>& dirSigns, Float tMax)
{
    // Check for ray intersection against $x$ and $y$ slabs
    Float txMin = (bbox[dirSigns[0]].x - origin.x) * invDir.x;
    Float txMax = (bbox[1 - dirSigns[0]].x - origin.x) * invDir.x;
    Float tyMin = (bbox[dirSigns[1]].y - origin.y) * invDir.y;
    Float tyMax = (bbox[1 - dirSigns[1]].y - origin.y) * invDir.y;
    // Update _tMax_ and _tyMax_ to ensure robust bbox intersection
    txMax *= 1 + 2 * gamma<Float>(3);
    tyMax *= 1 + 2 * gamma<Float>(3);

    if (txMin > tyMax || tyMin > txMax)
    {
        return false;
    }
    if (tyMin > txMin)
    {
        txMin = tyMin;
    }
    if (tyMax < txMax)
    {
        txMax = tyMax;
    }

    // Check for ray intersection against $z$ slab
    Float tzMin = (bbox[dirSigns[2]].z - origin.z) * invDir.z;
    Float tzMax = (bbox[1 - dirSigns[2]].z - origin.z) * invDir.z;
    // Update _tzMax_ to ensure robust bbox intersection
    tzMax *= 1 + 2 * gamma<Float>(3);

    if (txMin > tzMax || tzMin > txMax)
    {
        return false;
    }
    if (tzMin > txMin)
    {
        txMin = tzMin;
    }
    if (tzMax < txMax)
    {
        txMax = tzMax;
    }

    return (txMin < tMax) && (txMax > 0);
}
