#include <jate/application.h>

#include <spdlog/spdlog.h>

namespace jate
{
    Application::Application() 
        : m_window("My window", 800, 600), 
          m_vulkanInstance("My app"),
          m_vulkanDevice(m_vulkanInstance, m_window),
          m_vulkanSwapChain(m_window, m_vulkanDevice)
    {
        init_createPipelineLayout();
        init_createPipeline();
    }

    Application::~Application()
    {
        vkDestroyPipelineLayout(m_vulkanDevice.getVkDevice(), m_pipelineLayout, nullptr);
    }

    void Application::init_createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_vulkanDevice.getVkDevice(), &layoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		{
            spdlog::error("failed to create pipeline layout");
            return;
		}
    }

    void Application::init_createPipeline()
    {
        vulkan::VulkanPipeline::PipelineConfigInfo pipelineConfig {};
        vulkan::VulkanPipeline::PipelineConfigInfo::defaultConfig(pipelineConfig);
        pipelineConfig.renderPass = m_vulkanSwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_vulkanPipeline = std::make_unique<vulkan::VulkanPipeline>(m_vulkanDevice, "", "", pipelineConfig);    // TODO put in file paths for shaders
    }

    void Application::run()
    {
        m_running = true;

        // Main loop
        while (!m_window.shouldClose())
        {
            glfwWaitEvents();   // TODO use pollevents instead for main loop once we'll have actual frames, for now it used 100% CPU for nothing
        }
    }
}