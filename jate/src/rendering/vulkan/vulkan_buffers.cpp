#include <jate/rendering/vulkan/vulkan_buffers.h>

namespace jate::rendering::vulkan
{
	// --- AVulkanBuffer

	AVulkanBuffer::AVulkanBuffer(VulkanDevice& device, VkDeviceSize bufferOffset)
		: m_device(device), m_bufferOffset(bufferOffset)
	{
	}

	AVulkanBuffer::~AVulkanBuffer()
	{
		if (m_buffer != nullptr)
			vkDestroyBuffer(m_device.getVkDevice(), m_buffer, nullptr);
		
		if (m_bufferMemory != nullptr)
			vkFreeMemory(m_device.getVkDevice(), m_bufferMemory, nullptr);
	}

	void AVulkanBuffer::createStagingBuffer(VkBuffer& outBuffer, VkDeviceMemory& outBufferMemory, const void* bufferData, VkDeviceSize bufferSize)
	{
		// Create staging buffer, a temporary host-visible buffer
		m_device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			outBuffer, outBufferMemory
		);

		// Mapping staging device memory to host memory in order to write into it
		void* hostData;
		vkMapMemory(m_device.getVkDevice(), outBufferMemory, m_bufferOffset, bufferSize, 0, &hostData);

		// Copy data from the vertices vector to the host data, which is mapped to device memory.
		// Thanks to the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT flag, this will automatically be flushed to device memory
		memcpy(hostData, bufferData, static_cast<size_t>(bufferSize));

		// Release mapping
		vkUnmapMemory(m_device.getVkDevice(), outBufferMemory);
	}

	// --- VulkanVertexBuffer

    VulkanVertexBuffer::VulkanVertexBuffer(VulkanDevice& device, const std::vector<Vertex>& vertices, VkDeviceSize bufferOffset)
		: AVulkanBuffer(device, bufferOffset)
	{
		init_createVertexBuffer(vertices);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		// buffer and memory deletion happens in parent class 
	}

	void VulkanVertexBuffer::init_createVertexBuffer(const std::vector<Vertex>& vertices)
	{
		m_vertexCount = static_cast<uint32_t>(vertices.size());
		assert(m_vertexCount >= 3 && "VertexCount must be at least 3");
		VkDeviceSize bufferSize = sizeof(Vertex) * m_vertexCount;

		// Create staging buffer, a temporary host-visible buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;
		createStagingBuffer(stagingBuffer, stagingMemory, vertices.data(), bufferSize);

		// Create vertex buffer and its local device memory (only visible by device), using transfer queue to get data from staging buffer
		m_device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_buffer, m_bufferMemory
		);
		m_device.copyBuffer(stagingBuffer, m_buffer, bufferSize);

		// Destroy staging buffer
		vkDestroyBuffer(m_device.getVkDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_device.getVkDevice(), stagingMemory, nullptr);
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

	// --- VulkanIndexBuffer

    VulkanIndexBuffer::VulkanIndexBuffer(VulkanDevice &device, const std::vector<uint32_t> &indices, VkDeviceSize bufferOffset)
		: AVulkanBuffer(device, bufferOffset)
    {
		init_createIndexBuffer(indices);
    }

    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
		// buffer and memory deletion happens in parent class 
    }

    void VulkanIndexBuffer::init_createIndexBuffer(const std::vector<uint32_t> &indices)
    {
		m_indexCount = static_cast<uint32_t>(indices.size());
		assert(m_indexCount >= 3 && "IndexCount must be at least 3");
		VkDeviceSize bufferSize = sizeof(uint32_t) * m_indexCount;

		// Create staging buffer, a temporary host-visible buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;
		createStagingBuffer(stagingBuffer, stagingMemory, indices.data(), bufferSize);

		// Create vertex buffer and its local device memory (only visible by device), using transfer queue to get data from staging buffer
		m_device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_buffer, m_bufferMemory
		);
		m_device.copyBuffer(stagingBuffer, m_buffer, bufferSize);

		// Destroy staging buffer
		vkDestroyBuffer(m_device.getVkDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_device.getVkDevice(), stagingMemory, nullptr);
    }
}