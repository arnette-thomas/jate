#ifndef Jate_VulkanPipeline_H
#define Jate_VulkanPipeline_H

#include <jate/vulkan/vulkan_device.h>

namespace jate::vulkan
{
    class VulkanPipeline
    {
    public:
        struct PipelineConfigInfo
        {
            VkPipelineViewportStateCreateInfo viewportInfo;
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
            VkPipelineRasterizationStateCreateInfo rasterizationInfo;
            VkPipelineMultisampleStateCreateInfo multisampleInfo;
            VkPipelineColorBlendAttachmentState colorBlendAttachment;
            VkPipelineColorBlendStateCreateInfo colorBlendInfo;
            VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

            std::vector<VkDynamicState> dynamicStateEnables;
            VkPipelineDynamicStateCreateInfo dynamicStateInfo;

            VkPipelineLayout pipelineLayout = nullptr;
            VkRenderPass renderPass = nullptr;
            uint32_t subpass = 0;

            PipelineConfigInfo() = default;
            PipelineConfigInfo(const PipelineConfigInfo&) = delete;
            PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

            static void defaultConfig(PipelineConfigInfo& conf);
        };
        
        VulkanPipeline(VulkanDevice& device, const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& config);
		~VulkanPipeline();

		// No copy allowed
		VulkanPipeline(const VulkanPipeline&) = delete;
		VulkanPipeline& operator=(const VulkanPipeline&) = delete;

		void cmdBind(VkCommandBuffer commandBuffer);

    private:
		static std::vector<char> readFile(const std::string& path);

		void createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& config);
		void createShaderModule(const std::vector<char>& shaderCode, VkShaderModule* shaderModule);

		// Variables

		VulkanDevice& m_device; // The device should be loaded in memory as long as the pipeline exists
		VkPipeline m_graphicsPipeline;
		VkShaderModule m_vertShaderModule, m_fragShaderModule;
    };
} 

#endif