#ifndef VULKAN_PIPELINE_H
#define VULKAN_PIPELINE_H
#pragma once

#include "VulkanCommon.h"

class VulkanPipelineLayout : public VulkanObject
{
public:
    VulkanPipelineLayout(Ref<VulkanDevice> device, const VkPipelineLayoutCreateInfo& createInfo);
    ~VulkanPipelineLayout();

    VkPipelineLayout GetHandle() const { return m_handle; }

private:
    Ref<VulkanDevice>       m_device;
    VkPipelineLayout        m_handle = VK_NULL_HANDLE;

}; // class VulkanPipelineLayout

// @TODO: PipelineCache
class VulkanPipeline : public VulkanObject
{
public:
    VulkanPipeline(Ref<VulkanDevice> device, VkPipelineBindPoint bindPoint);
    ~VulkanPipeline();

    operator VkPipeline() const { return m_handle; }
    VkPipeline GetHandle() const { return m_handle; }

    VkPipelineBindPoint GetBindPoint() const;

protected:
    bool SaveCacheToFile(VkPipelineCache cache, std::string_view fileName);

    Ref<VulkanDevice>       m_device;
    VkPipeline              m_handle = VK_NULL_HANDLE;
    VkPipelineBindPoint     m_bindPoint;

}; // class VulkanPipeline

class VulkanGraphicsPipeline : public VulkanPipeline
{
public:
    VulkanGraphicsPipeline(Ref<VulkanDevice> device, const VulkanGraphicsPipelineCreateInfo& createInfo);

}; // class VulkanGraphicsPipeline

class VulkanComputePipeline : public VulkanPipeline
{
public:
    VulkanComputePipeline(Ref<VulkanDevice> device, const VulkanComputePipelineCreateInfo& createInfo);

}; // class VulkanComputePipeline

#endif // VULKAN_PIPELINE_H