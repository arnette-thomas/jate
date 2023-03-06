#ifndef Jate_Transform_H
#define Jate_Transform_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <jate/maths/vectors.h>
#include <jate/maths/quaternions.h>

namespace jate::models
{
    class Transform
    {
    public:
        jate::maths::Vector3f position;
        jate::maths::Quaternionf rotation;
        jate::maths::Vector3f scale;

        Transform(jate::maths::Vector3f _position, jate::maths::Quaternionf _rotation, jate::maths::Vector3f _scale) : position(_position), rotation(_rotation), scale(_scale) {}

        glm::mat4 getMatrix() const
        {
            return glm::mat4{
                {
                    scale.x * (1.f - 2.f * rotation.y * rotation.y - 2.f * rotation.z * rotation.z),
                    scale.x * (2.f * rotation.x * rotation.y + 2.f * rotation.w * rotation.z),
                    scale.x * (2.f * rotation.x * rotation.z - 2.f * rotation.w * rotation.y),
                    0.f
                },
                {
                    scale.x * (2.f * rotation.x * rotation.y - 2.f * rotation.w * rotation.z),
                    scale.y * (1.f - 2.f * rotation.x * rotation.x - 2.f * rotation.z * rotation.z),
                    scale.z * (2.f * rotation.y * rotation.z + 2.f * rotation.w * rotation.x),
                    0.f
                },
                {
                    scale.x * (2.f * rotation.x * rotation.z - 2.f * rotation.w * rotation.y),
                    scale.y * (2.f * rotation.y * rotation.z - 2.f * rotation.w * rotation.x),
                    scale.z * (1.f - 2.f * rotation.x * rotation.x + 2.f * rotation.y * rotation.y),
                    0.f
                },
                {
                    position.x,
                    position.y,
                    position.z,
                    1.f
                }
            };
        }
    };
}

#endif
