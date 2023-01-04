#ifndef VULKAN_SAMPLER_H
#define VULKAN_SAMPLER_H
#pragma once

#include "VulkanCommon.h"

class VulkanSampler : public VulkanObject
{
public:
    VulkanSampler(Ref<VulkanDevice> device, const VkSamplerCreateInfo& createInfo);
    ~VulkanSampler();

    VkSampler GetHandle() const { return m_handle; }

private:
    Ref<VulkanDevice> m_device;
    VkSampler m_handle = VK_NULL_HANDLE;

}; // class VulkanSampler

#endif // VULKAN_SAMPLER_H