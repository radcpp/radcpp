#ifndef VULKAN_COMMAND_POOL_H
#define VULKAN_COMMAND_POOL_H
#pragma once

#include "VulkanCommon.h"

class VulkanCommandPool : public VulkanObject
{
public:
    VulkanCommandPool(Ref<VulkanDevice> device, const VkCommandPoolCreateInfo& createInfo);
    ~VulkanCommandPool();

    VkCommandPool GetHandle() const { return m_handle; }

    Ref<VulkanCommandBuffer> Allocate(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Trimming a command pool recycles unused memory from the command pool back to the system.
    // Command buffers allocated from the pool are not affected by the command.
    void Trim();
    // Resetting a command pool recycles all of the resources -
    // from all of the command buffers allocated from the command pool back to the command pool.
    // All command buffers that have been allocated from the command pool are put in the initial state.
    void Reset(VkCommandPoolResetFlags flags = 0);

private:
    Ref<VulkanDevice>           m_device;
    VkCommandPool               m_handle = VK_NULL_HANDLE;

}; // class VulkanCommandPool

#endif // VULKAN_COMMAND_POOL_H