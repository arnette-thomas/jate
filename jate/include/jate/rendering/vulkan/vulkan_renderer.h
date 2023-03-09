#ifndef Jate_VulkanRenderer_H
#define Jate_VulkanRenderer_H

#include <jate/rendering/renderer.h>

#include <jate/rendering/vulkan/vulkan_instance.h>
#include <jate/rendering/vulkan/vulkan_device.h>
#include <jate/rendering/vulkan/vulkan_swapchain.h>
#include <jate/rendering/vulkan/vulkan_pipeline.h>
#include <jate/rendering/vulkan/vulkan_command_manager.h>

#include <memory>
#include <unordered_map>

namespace jate::rendering::vulkan
{
    class VulkanRenderer : public ARenderer
    {
    public:
        VulkanRenderer(Window& window);
        ~VulkanRenderer();


        virtual renderer_memory_slot_id allocateVertexData(const std::vector<VertexData>& vertices) override;
        virtual void freeVertexData(renderer_memory_slot_id slotId);

        virtual renderer_memory_slot_id allocateIndexData(const std::vector<uint32_t>& indices) override;
        virtual void freeIndexData(renderer_memory_slot_id slotId);

        virtual void drawIndexed(renderer_memory_slot_id verticesSlotId, renderer_memory_slot_id indicesSlotId, const PushConstantData& pushConstantData) override;

    private:
        void init_createSwapChain();
        void init_createCommandManager();
        void init_createPipelineLayout();
        void init_createPipeline();
        void init_createSyncObjects();

        void recreateSwapChain();

        virtual void beginFrame() override;
        virtual void endFrame() override;

        VulkanInstance m_vulkanInstance;
        VulkanDevice m_vulkanDevice;
        std::unique_ptr<VulkanSwapChain> m_vulkanSwapChain;
        std::unique_ptr<VulkanCommandManager> m_vulkanCommandManager;

        std::unique_ptr<VulkanPipeline> m_vulkanPipeline;
        VkPipelineLayout m_pipelineLayout;

        // Sync objects
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;

        uint32_t m_currentImageIndex;
        VulkanCommandBuffer* m_currentFrameCommandBuffer = nullptr;

        const uint8_t MAX_FRAMES_IN_FLIGHT = 2;
        uint8_t m_currentFrameInFlight = 0;

        // Renderer memory slots
        std::unordered_map<renderer_memory_slot_id, std::unique_ptr<VulkanVertexBuffer>> m_vertexBufferSlots;
        std::unordered_map<renderer_memory_slot_id, std::unique_ptr<VulkanIndexBuffer>> m_indexBufferSlots;
    };
}

#endif