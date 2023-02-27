#ifndef Jate_VulkanVertexBuffer_H
#define Jate_VulkanVertexBuffer_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <jate/vulkan/vulkan_device.h>

namespace jate::vulkan
{
    class VulkanVertexBuffer
    {
    public:

		struct Vertex
		{
			glm::vec2 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		VulkanVertexBuffer(VulkanDevice& device, const std::vector<Vertex>& vertices);
		~VulkanVertexBuffer();

		// No copy allowed
		VulkanVertexBuffer(const VulkanVertexBuffer&) = delete;
		VulkanVertexBuffer& operator=(const VulkanVertexBuffer&) = delete;

		void cmdBind(VkCommandBuffer commandBuffer);
		void cmdDraw(VkCommandBuffer commandBuffer);
	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);

		VulkanDevice& m_device;

		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;
		uint32_t m_vertexCount;
    };
}

#endif