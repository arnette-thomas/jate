#include <jate/vulkan/vulkan_pipeline.h>

#include <jate/vulkan/vulkan_vertexbuffer.h>

#include <fstream>

namespace jate::vulkan
{
    VulkanPipeline::VulkanPipeline(VulkanDevice& device, const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& config)
		: m_device(device)
	{
		createGraphicsPipeline(vertFilePath, fragFilePath, config);
	}

	VulkanPipeline::~VulkanPipeline()
	{
		vkDestroyShaderModule(m_device.getVkDevice(), m_vertShaderModule, nullptr);
		vkDestroyShaderModule(m_device.getVkDevice(), m_fragShaderModule, nullptr);
		vkDestroyPipeline(m_device.getVkDevice(), m_graphicsPipeline, nullptr);
	}

	void VulkanPipeline::cmdBind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
	}

	std::vector<char> VulkanPipeline::readFile(const std::string& path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file " + path + " !");
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	void VulkanPipeline::createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& config)
	{
		auto vertCode = readFile(vertFilePath);
		auto fragCode = readFile(fragFilePath);

		/*std::cout << "vert code size : " << vertCode.size() << std::endl;
		std::cout << "frag code size : " << fragCode.size() << std::endl;*/

		createShaderModule(vertCode, &m_vertShaderModule);
		createShaderModule(fragCode, &m_fragShaderModule);

		VkPipelineShaderStageCreateInfo shaderStages[2];
		// Vertex stage
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = m_vertShaderModule;
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;
		// Fragment stage
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = m_fragShaderModule;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;

		// Setup vertex input
		auto bindingDescriptions = VulkanVertexBuffer::Vertex::getBindingDescriptions();
		auto attributeDescriptions = VulkanVertexBuffer::Vertex::getAttributeDescriptions();
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		// Setup graphics pipeline
		VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
		graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		graphicsPipelineInfo.stageCount = 2;
		graphicsPipelineInfo.pStages = shaderStages;
		graphicsPipelineInfo.pVertexInputState = &vertexInputInfo;

		graphicsPipelineInfo.pViewportState = &config.viewportInfo;
		graphicsPipelineInfo.pInputAssemblyState = &config.inputAssemblyInfo;
		graphicsPipelineInfo.pRasterizationState = &config.rasterizationInfo;
		graphicsPipelineInfo.pMultisampleState = &config.multisampleInfo;
		graphicsPipelineInfo.pColorBlendState = &config.colorBlendInfo;
		graphicsPipelineInfo.pDepthStencilState = &config.depthStencilInfo;
		graphicsPipelineInfo.pDynamicState = &config.dynamicStateInfo;

		graphicsPipelineInfo.renderPass = config.renderPass;
		graphicsPipelineInfo.subpass = config.subpass;
		graphicsPipelineInfo.layout = config.pipelineLayout;

		graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		graphicsPipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(m_device.getVkDevice(), VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline");
		};
	}

	void VulkanPipeline::createShaderModule(const std::vector<char>& shaderCode, VkShaderModule* shaderModule)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
		createInfo.flags = 0;

		if (vkCreateShaderModule(m_device.getVkDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
		{
			throw new std::runtime_error("failed to create shader module");
		}
	}


	void VulkanPipeline::PipelineConfigInfo::defaultConfig(PipelineConfigInfo& conf)
	{
		conf.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		conf.viewportInfo.viewportCount = 1;
		conf.viewportInfo.pViewports = nullptr;	// Dynamic
		conf.viewportInfo.scissorCount = 1;
		conf.viewportInfo.pScissors = nullptr;	// Dynamic

		// Input assembly stage => makes input for the Vertex shader
		conf.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		conf.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		conf.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// Rasterization stage => breaks up geometry into fragments
		conf.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		conf.rasterizationInfo.depthClampEnable = VK_FALSE;			// Disabling clamping depth values between 0 and 1
		conf.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		conf.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		conf.rasterizationInfo.lineWidth = 1.0f;
		conf.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;		// Showing all triangles, regardless of their facing
		conf.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;	// Clockwise vertices define a front-facing triangle, and Counter-Clockwise vertices define a back-facing triangle
		conf.rasterizationInfo.depthBiasEnable = VK_FALSE;
		conf.rasterizationInfo.depthBiasClamp = 0.0f;			// optional since disabled
		conf.rasterizationInfo.depthBiasConstantFactor = 0.0f;	// optional since disabled
		conf.rasterizationInfo.depthBiasSlopeFactor = 0.0f;		// optional since disabled

		// Multisampling defines how rasterization handles the edges of the geometry (to prevent aliasing).
		// It helps to compute how much of a fragment is contained in a triangle
		conf.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		conf.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		conf.multisampleInfo.alphaToOneEnable = VK_FALSE;
		conf.multisampleInfo.minSampleShading = 1.0f;
		conf.multisampleInfo.sampleShadingEnable = VK_FALSE;
		conf.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		conf.multisampleInfo.pSampleMask = nullptr;

		// Color blending stage => defines how colors are combined in the frame buffer
		conf.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		conf.colorBlendAttachment.blendEnable = VK_FALSE;
		conf.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		conf.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		conf.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		conf.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		conf.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		conf.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		conf.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		conf.colorBlendInfo.logicOpEnable = VK_FALSE;
		conf.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		conf.colorBlendInfo.attachmentCount = 1;
		conf.colorBlendInfo.pAttachments = &conf.colorBlendAttachment;
		conf.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		conf.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		conf.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		conf.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		// Depth testing configuration
		conf.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		conf.depthStencilInfo.depthTestEnable = VK_TRUE;
		conf.depthStencilInfo.depthWriteEnable = VK_TRUE;
		conf.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		conf.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		conf.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
		conf.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
		conf.depthStencilInfo.stencilTestEnable = VK_FALSE;
		conf.depthStencilInfo.front = {};  // Optional
		conf.depthStencilInfo.back = {};   // Optional

		// Dynamic states
		conf.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		conf.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		conf.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(conf.dynamicStateEnables.size());
		conf.dynamicStateInfo.pDynamicStates = conf.dynamicStateEnables.data();
		conf.dynamicStateInfo.flags = 0;
	}
}