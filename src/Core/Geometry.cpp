#include "rad/Core/Geometry.h"

// https://pbr-book.org/3ed-2018/Geometry_and_Transformations/Vectors
void ConstructCoordinateSystem(const glm::vec3& v1, glm::vec3& v2, glm::vec3& v3)
{
    if (std::abs(v1.x) > std::abs(v1.y))
    {
        v2 = glm::vec3(-v1.z, 0, v1.x) /
            glm::sqrt(v1.x * v1.x + v1.z * v1.z);
    }
    else
    {
        v2 = glm::vec3(0, v1.z, -v1.y) /
            glm::sqrt(v1.y * v1.y + v1.z * v1.z);
    }
    v3 = glm::cross(v1, v2);
}
