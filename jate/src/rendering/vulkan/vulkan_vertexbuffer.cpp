#include <jate/rendering/vulkan/vulkan_vertexbuffer.h>

namespace jate::rendering::vulkan
{
    VulkanVertexBuffer::VulkanVertexBuffer(VulkanDevice& device, const std::vector<Vertex>& vertices)
		: m_device(device)
	{
		createVertexBuffers(vertices);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		vkDestroyBuffer(m_device.getVkDevice(), m_vertexBuffer, nullptr);
		vkFreeMemory(m_device.getVkDevice(), m_vertexBufferMemory, nullptr);
	}

	void VulkanVertexBuffer::cmdBind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { m_vertexBuffer };
		VkDeviceSize bufferOffsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, bufferOffsets);
	}

	void VulkanVertexBuffer::cmdDraw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
	}

	void VulkanVertexBuffer::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_vertexCount = static_cast<uint32_t>(vertices.size());
		assert(m_vertexCount >= 3 && "VertexCount must be at least 3");
		VkDeviceSize bufferSize = sizeof(Vertex) * m_vertexCount;

		// Create vertex buffer and its device memory
		m_device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_vertexBuffer, m_vertexBufferMemory
		);

		// Mapping vertex device memory to host memory in order to write into it
		void* hostData;
		vkMapMemory(m_device.getVkDevice(), m_vertexBufferMemory, 0, bufferSize, 0, &hostData);

		// Copy data from the vertices vector to the host data, which is mapped to device memory.
		// Thanks to the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT flag, this will automatically be flushed to device memory
		memcpy(hostData, vertices.data(), static_cast<size_t>(bufferSize));

		// Release mapping
		vkUnmapMemory(m_device.getVkDevice(), m_vertexBufferMemory);
	}

	std::vector<VkVertexInputBindingDescription> VulkanVertexBuffer::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescriptions[0].stride = sizeof(Vertex);
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> VulkanVertexBuffer::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

		// Position
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].offset = offsetof(Vertex, position);
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;

		// Color
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;

		return attributeDescriptions;
	}
}