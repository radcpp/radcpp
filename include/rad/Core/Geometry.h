#pragma once

#include "rad/Core/Common.h"
#include "rad/Core/Math.h"

#include "glm/glm.hpp"
#include "glm/ext.hpp"

/// Construct coordinate system from a single 3D vector.
/// v1 must be normalized.
void ConstructCoordinateSystem(const glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);
