#ifndef VULKAN_SEMAPHORE_H
#define VULKAN_SEMAPHORE_H
#pragma once

#include "VulkanCommon.h"

// GPU-GPU sync object
class VulkanSemaphore : public VulkanObject
{
public:
    VulkanSemaphore(Ref<VulkanDevice> device, const VkSemaphoreCreateInfo& createInfo);
    VulkanSemaphore(Ref<VulkanDevice> device, VkSemaphore handle);
    ~VulkanSemaphore();

    VkSemaphore GetHandle() const { return m_handle; }

private:
    Ref<VulkanDevice> m_device;
    VkSemaphore m_handle = VK_NULL_HANDLE;
    bool m_isManaged = true;

}; // class VulkanSemaphore

#endif // VULKAN_SEMAPHORE_H