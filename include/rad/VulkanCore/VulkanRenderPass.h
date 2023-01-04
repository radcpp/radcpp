#ifndef VULKAN_RENDER_PASS_H
#define VULKAN_RENDER_PASS_H
#pragma once

#include "VulkanCommon.h"

class VulkanRenderPass : public VulkanObject
{
public:
    VulkanRenderPass(Ref<VulkanDevice> device, const VkRenderPassCreateInfo& createInfo);
    ~VulkanRenderPass();

    VkRenderPass GetHandle() const { return m_handle; }

private:
    Ref<VulkanDevice> m_device;
    VkRenderPass m_handle = VK_NULL_HANDLE;

}; // class VulkanRenderPass

#endif // VULKAN_RENDER_PASS_H