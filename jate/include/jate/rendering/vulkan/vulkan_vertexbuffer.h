#ifndef Jate_VulkanVertexBuffer_H
#define Jate_VulkanVertexBuffer_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <jate/rendering/vulkan/vulkan_device.h>

namespace jate::rendering::vulkan
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

		VulkanVertexBuffer(VulkanDevice& device, const std::vector<Vertex>& vertices, VkDeviceSize bufferOffset = 0);
		~VulkanVertexBuffer();

		// No copy allowed
		VulkanVertexBuffer(const VulkanVertexBuffer&) = delete;
		VulkanVertexBuffer& operator=(const VulkanVertexBuffer&) = delete;

		inline VkBuffer getVkBuffer() const { return m_vertexBuffer; }
		inline VkDeviceSize getBufferOffset() const { return m_bufferOffset; }
		inline uint32_t getVertexCount() const { return m_vertexCount; }

	private:
		void init_createVertexBuffers(const std::vector<Vertex>& vertices);

		VulkanDevice& m_device;

		VkDeviceSize m_bufferOffset;
		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;
		uint32_t m_vertexCount;
    };
}

#endif