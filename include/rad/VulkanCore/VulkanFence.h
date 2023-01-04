#ifndef VULKAN_FENCE_H
#define VULKAN_FENCE_H
#pragma once

#include "VulkanCommon.h"

// GPU-CPU sync
class VulkanFence : public VulkanObject
{
public:
    VulkanFence(Ref<VulkanDevice> device, const VkFenceCreateInfo& createInfo);
    ~VulkanFence();

    VkFence GetHandle() const { return m_handle; }

    // in nanoseconds, will be adjusted to the closest value allowed by the implementation dependent timeout accuracy.
    void Wait(uint64_t timeout = UINT64_MAX);
    void Reset();

private:
    Ref<VulkanDevice> m_device;
    VkFence m_handle = VK_NULL_HANDLE;

}; // class VulkanFence

#endif // VULKAN_FENCE_H