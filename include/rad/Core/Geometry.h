#pragma once

#include "rad/Core/Common.h"
#include "rad/Core/Math.h"

#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include <numeric>

/// Construct coordinate system from a single 3D vector.
/// v1 must be normalized.
void ConstructCoordinateSystem(const glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);

template<typename T>
using Point2D = glm::vec<2, T, glm::defaultp>;
template<typename T>
using Point3D = glm::vec<3, T, glm::defaultp>;

template<typename T>
using Vector2D = glm::vec<2, T, glm::defaultp>;
template<typename T>
using Vector3D = glm::vec<3, T, glm::defaultp>;
