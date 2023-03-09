#ifndef Jate_VulkanVertexBuffer_H
#define Jate_VulkanVertexBuffer_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <jate/rendering/vulkan/vulkan_device.h>
#include <jate/rendering/data_structs.h>

namespace jate::rendering::vulkan
{
	class AVulkanBuffer
	{
	public:
		virtual ~AVulkanBuffer();

		// No copy allowed - maybe implement that later ?
        AVulkanBuffer(const AVulkanBuffer&) = delete;
        AVulkanBuffer& operator=(const AVulkanBuffer&) = delete;

		inline VkBuffer getVkBuffer() const { return m_buffer; }
		inline VkDeviceSize getBufferOffset() const { return m_bufferOffset; }

	protected:
		AVulkanBuffer(VulkanDevice& device, VkDeviceSize bufferOffset = 0);

		void createStagingBuffer(VkBuffer& outBuffer, VkDeviceMemory& outBufferMemory, const void* bufferData, VkDeviceSize bufferSize);

		VulkanDevice& m_device;
		VkDeviceSize m_bufferOffset = 0;
		VkBuffer m_buffer;
		VkDeviceMemory m_bufferMemory;
	};

    class VulkanVertexBuffer : public AVulkanBuffer
    {
    public:
		VulkanVertexBuffer(VulkanDevice& device, const std::vector<VertexData>& vertices, VkDeviceSize bufferOffset = 0);
		virtual ~VulkanVertexBuffer();

		inline uint32_t getVertexCount() const { return m_vertexCount; }

		static std::vector<VkVertexInputBindingDescription> getVertexBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions();

	private:
		void init_createVertexBuffer(const std::vector<VertexData>& vertices);

		uint32_t m_vertexCount;
    };

	class VulkanIndexBuffer : public AVulkanBuffer
	{
	public:
		VulkanIndexBuffer(VulkanDevice& device, const std::vector<uint32_t>& indices, VkDeviceSize bufferOffset = 0);
		virtual ~VulkanIndexBuffer();

		inline uint32_t getIndexCount() const { return m_indexCount; }

	private:
		void init_createIndexBuffer(const std::vector<uint32_t>& indices);

		uint32_t m_indexCount;
	};
}

#endif