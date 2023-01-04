#include "rad/VulkanCore/VulkanPipeline.h"
#include "rad/VulkanCore/VulkanDevice.h"
#include "rad/VulkanCore/VulkanShader.h"
#include "rad/VulkanCore/VulkanRenderPass.h"

#include "rad/Core/File.h"

VulkanPipelineLayout::VulkanPipelineLayout(Ref<VulkanDevice> device, const VkPipelineLayoutCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreatePipelineLayout(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
    m_device->GetFunctionTable()->
        vkDestroyPipelineLayout(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}

VulkanPipeline::VulkanPipeline(Ref<VulkanDevice> device, VkPipelineBindPoint bindPoint) :
    m_device(std::move(device)),
    m_bindPoint(bindPoint)
{
}

VulkanPipeline::~VulkanPipeline()
{
    m_device->GetFunctionTable()->
        vkDestroyPipeline(m_device->GetHandle(), m_handle, nullptr);
}

VkPipelineBindPoint VulkanPipeline::GetBindPoint() const
{
    return m_bindPoint;
}

bool VulkanPipeline::SaveCacheToFile(VkPipelineCache cache, std::string_view fileName)
{
    size_t cacheDataSize = 0;

    VK_CHECK(m_device->GetFunctionTable()->
        vkGetPipelineCacheData(m_device->GetHandle(), cache, &cacheDataSize, nullptr));

    if (cacheDataSize > 0)
    {
        std::vector<uint8_t> cacheData;
        cacheData.resize(cacheDataSize);
        VK_CHECK(m_device->GetFunctionTable()->
            vkGetPipelineCacheData(m_device->GetHandle(), cache, &cacheDataSize, cacheData.data()));

        File file;
        if (file.Open(fileName, FileOpenWrite))
        {
            file.Write(cacheData.data(), cacheDataSize);
            file.Close();
            return true;
        }
    }
    return false;
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(Ref<VulkanDevice> device, const VulkanGraphicsPipelineCreateInfo& createInfo) :
    VulkanPipeline(std::move(device), VK_PIPELINE_BIND_POINT_GRAPHICS)
{
    std::vector<Ref<VulkanShaderModule>>        shaderModules;

    std::vector<VkDynamicState> dynamicState =
    {
        VK_DYNAMIC_STATE_VIEWPORT,  // pViewports state will be ignored and must be set dynamically with vkCmdSetViewport before any draw commands.
        VK_DYNAMIC_STATE_SCISSOR,   // pScissors state in VkPipelineViewportStateCreateInfo will be ignored and must be set dynamically with vkCmdSetScissor before any draw commands.
    };

    std::vector<VkPipelineShaderStageCreateInfo>    apiShaderStages;
    std::vector<VkSpecializationInfo>               apiSpecializationInfos;
    VkPipelineVertexInputStateCreateInfo            apiVertexInput;
    VkPipelineInputAssemblyStateCreateInfo          apiInputAssembly;
    VkPipelineTessellationStateCreateInfo           apiTessellation;
    VkPipelineViewportStateCreateInfo               apiViewportState;
    VkPipelineRasterizationStateCreateInfo          apiRasterizationState;
    VkPipelineMultisampleStateCreateInfo            apiMultisampleState;
    VkPipelineDepthStencilStateCreateInfo           apiDepthStencilState;
    VkPipelineColorBlendStateCreateInfo             apiColorBlendState;
    VkPipelineDynamicStateCreateInfo                apiDynamicState;

    VkGraphicsPipelineCreateInfo apiCreateInfo;
    apiCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    apiCreateInfo.pNext = nullptr;
    apiCreateInfo.flags = 0;

    for (const auto& shader : createInfo.m_shaders)
    {
        assert(shader->IsValid());
        Ref<VulkanShaderModule> shaderModule = m_device->CreateShaderModule(shader->GetBinary());
        shaderModules.push_back(shaderModule);
        VkPipelineShaderStageCreateInfo apiShaderStage = {};
        apiShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        apiShaderStage.pNext;
        apiShaderStage.flags;
        apiShaderStage.stage = shader->GetStage();
        apiShaderStage.module = shaderModule->GetHandle();
        apiShaderStage.pName = shader->GetEntryPoint();
        if (shader->GetLanguage() == ShaderLanguage::GLSL)
        {
            apiShaderStage.pName = "main";
        }
        apiShaderStage.pSpecializationInfo = nullptr;
        apiShaderStages.push_back(apiShaderStage);
    }

    apiCreateInfo.stageCount = static_cast<uint32_t>(apiShaderStages.size());
    apiCreateInfo.pStages = apiShaderStages.data();

    apiVertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    apiVertexInput.pNext = nullptr;
    apiVertexInput.flags = 0;
    apiVertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(createInfo.m_vertexInputState.bindings.size());
    apiVertexInput.pVertexBindingDescriptions = createInfo.m_vertexInputState.bindings.data();
    apiVertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(createInfo.m_vertexInputState.attributes.size());
    apiVertexInput.pVertexAttributeDescriptions = createInfo.m_vertexInputState.attributes.data();
    apiCreateInfo.pVertexInputState = &apiVertexInput;

    apiInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    apiInputAssembly.pNext = nullptr;
    apiInputAssembly.flags = 0;
    apiInputAssembly.topology = createInfo.m_inputAssemblyState.topology;
    apiInputAssembly.primitiveRestartEnable = createInfo.m_inputAssemblyState.primitiveRestartEnable;
    apiCreateInfo.pInputAssemblyState = &apiInputAssembly;

    VkShaderStageFlags pipelineStageFlags = 0;
    for (const auto& shader : createInfo.m_shaders)
    {
        pipelineStageFlags |= shader->GetStage();
    }

    if (HasAnyBit(pipelineStageFlags, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))
    {
        apiTessellation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        apiTessellation.pNext = nullptr;
        apiTessellation.flags = 0;
        apiTessellation.patchControlPoints = createInfo.m_tessellationState.patchControlPoints;
        apiCreateInfo.pTessellationState = &apiTessellation;
    }

    apiViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    apiViewportState.pNext = nullptr;
    apiViewportState.flags = 0;
    apiViewportState.viewportCount = createInfo.m_viewportCount;
    apiViewportState.pViewports = nullptr; // will be set dynamically
    apiViewportState.scissorCount = createInfo.m_scissorCount;
    apiViewportState.pScissors = nullptr; // will be set dynamically
    apiCreateInfo.pViewportState = &apiViewportState;

    apiRasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    apiRasterizationState.pNext = nullptr;
    apiRasterizationState.flags = 0; // is reserved for future use.
    apiRasterizationState.depthClampEnable = createInfo.m_rasterizationState.depthClampEnable;
    apiRasterizationState.rasterizerDiscardEnable = createInfo.m_rasterizationState.rasterizerDiscardEnable;
    apiRasterizationState.polygonMode = createInfo.m_rasterizationState.polygonMode;
    apiRasterizationState.cullMode = createInfo.m_rasterizationState.cullMode;
    apiRasterizationState.frontFace = createInfo.m_rasterizationState.frontFace;
    apiRasterizationState.depthBiasEnable = createInfo.m_rasterizationState.depthBiasEnable;
    apiRasterizationState.depthBiasConstantFactor = createInfo.m_rasterizationState.depthBiasConstantFactor;
    apiRasterizationState.depthBiasClamp = createInfo.m_rasterizationState.depthBiasClamp;
    apiRasterizationState.depthBiasSlopeFactor = createInfo.m_rasterizationState.depthBiasSlopeFactor;
    apiRasterizationState.lineWidth = createInfo.m_rasterizationState.lineWidth;
    apiCreateInfo.pRasterizationState = &apiRasterizationState;

    apiMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    apiMultisampleState.pNext = nullptr;
    apiMultisampleState.flags = 0;
    apiMultisampleState.rasterizationSamples = createInfo.m_multisampleState.rasterizationSamples;
    apiMultisampleState.sampleShadingEnable = createInfo.m_multisampleState.sampleShadingEnable;
    apiMultisampleState.minSampleShading = createInfo.m_multisampleState.minSampleShading;
    apiMultisampleState.pSampleMask = &createInfo.m_multisampleState.sampleMask;
    apiMultisampleState.alphaToCoverageEnable = createInfo.m_multisampleState.alphaToCoverageEnable;
    apiMultisampleState.alphaToOneEnable = createInfo.m_multisampleState.alphaToOneEnable;
    apiCreateInfo.pMultisampleState = &apiMultisampleState;

    apiDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    apiDepthStencilState.pNext = nullptr;
    apiDepthStencilState.flags = 0;
    apiDepthStencilState.depthTestEnable = createInfo.m_depthStencilState.depthTestEnable;
    apiDepthStencilState.depthWriteEnable = createInfo.m_depthStencilState.depthWriteEnable;
    apiDepthStencilState.depthCompareOp = createInfo.m_depthStencilState.depthCompareOp;
    apiDepthStencilState.depthBoundsTestEnable = createInfo.m_depthStencilState.depthBoundsTestEnable;
    apiDepthStencilState.stencilTestEnable = createInfo.m_depthStencilState.stencilTestEnable;
    apiDepthStencilState.front = createInfo.m_depthStencilState.front;
    apiDepthStencilState.back = createInfo.m_depthStencilState.back;
    apiDepthStencilState.minDepthBounds = createInfo.m_depthStencilState.minDepthBounds;
    apiDepthStencilState.maxDepthBounds = createInfo.m_depthStencilState.maxDepthBounds;
    apiCreateInfo.pDepthStencilState = &apiDepthStencilState;

    apiColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    apiColorBlendState.pNext = nullptr;
    apiColorBlendState.flags = 0;
    apiColorBlendState.logicOpEnable = createInfo.m_colorBlendState.logicOpEnable;
    apiColorBlendState.logicOp = createInfo.m_colorBlendState.logicOp;
    apiColorBlendState.attachmentCount = static_cast<uint32_t>(createInfo.m_colorBlendState.attachments.size());
    apiColorBlendState.pAttachments = createInfo.m_colorBlendState.attachments.data();
    apiColorBlendState.blendConstants[0] = createInfo.m_colorBlendState.blendConstants[0];
    apiColorBlendState.blendConstants[1] = createInfo.m_colorBlendState.blendConstants[1];
    apiColorBlendState.blendConstants[2] = createInfo.m_colorBlendState.blendConstants[2];
    apiColorBlendState.blendConstants[3] = createInfo.m_colorBlendState.blendConstants[3];
    apiCreateInfo.pColorBlendState = &apiColorBlendState;

    apiDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    apiDynamicState.pNext = nullptr;
    apiDynamicState.flags = 0;
    apiDynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicState.size());
    apiDynamicState.pDynamicStates = dynamicState.data();

    apiCreateInfo.pDynamicState = &apiDynamicState;
    apiCreateInfo.layout = createInfo.m_layout ? createInfo.m_layout->GetHandle() : VK_NULL_HANDLE;
    apiCreateInfo.renderPass = createInfo.m_renderPass->GetHandle();
    apiCreateInfo.subpass = createInfo.m_subpass;
    apiCreateInfo.basePipelineHandle = createInfo.m_basePipeline ? createInfo.m_basePipeline->GetHandle() : VK_NULL_HANDLE;
    apiCreateInfo.basePipelineIndex = createInfo.m_basePipelineIndex;

    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateGraphicsPipelines(m_device->GetHandle(), VK_NULL_HANDLE, 1, &apiCreateInfo, nullptr, &m_handle));
}

VulkanComputePipeline::VulkanComputePipeline(Ref<VulkanDevice> device, const VulkanComputePipelineCreateInfo& createInfo) :
    VulkanPipeline(std::move(device), VK_PIPELINE_BIND_POINT_COMPUTE)
{
    Ref<VulkanShaderModule> shaderModule = m_device->CreateShaderModule(createInfo.shader->GetBinary());

    VkComputePipelineCreateInfo apiCreateInfo = {};
    apiCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    apiCreateInfo.pNext = nullptr;
    apiCreateInfo.flags = 0;
    apiCreateInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    apiCreateInfo.stage.pNext = nullptr;
    apiCreateInfo.stage.flags = 0;
    apiCreateInfo.stage.stage = createInfo.shader->GetStage();
    apiCreateInfo.stage.module = shaderModule->GetHandle();
    apiCreateInfo.stage.pName = createInfo.shader->GetEntryPoint();
    apiCreateInfo.stage.pSpecializationInfo = nullptr;;
    apiCreateInfo.layout = createInfo.layout ? createInfo.layout->GetHandle() : VK_NULL_HANDLE;
    apiCreateInfo.basePipelineHandle = createInfo.basePipeline ? createInfo.basePipeline->GetHandle() : VK_NULL_HANDLE;
    apiCreateInfo.basePipelineIndex = createInfo.basePipelineIndex;
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateComputePipelines(m_device->GetHandle(), VK_NULL_HANDLE, 1, &apiCreateInfo, nullptr, &m_handle));
}
