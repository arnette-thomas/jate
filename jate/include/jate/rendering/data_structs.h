#ifndef Jate_RenderingDataStructs_H
#define Jate_RenderingDataStructs_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace jate::rendering
{
    struct VertexData
    {
        glm::vec3 position;
		glm::vec3 color;
    };

    struct PushConstantData
    {
        glm::mat4 transform;
    };
}

#endif