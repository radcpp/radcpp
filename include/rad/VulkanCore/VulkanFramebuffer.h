#ifndef VULKAN_FRAMEBUFFER_H
#define VULKAN_FRAMEBUFFER_H
#pragma once

#include "VulkanCommon.h"

class VulkanFramebuffer : public VulkanObject
{
public:
    VulkanFramebuffer(Ref<VulkanDevice> device, const VkFramebufferCreateInfo& createInfo);
    ~VulkanFramebuffer();

    VkFramebuffer GetHandle() const { return m_handle; }

    uint32_t GetAttachmentCount() const { return m_attachmentCount; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    uint32_t GetLayers() const { return m_layers; }

private:
    Ref<VulkanDevice>       m_device;
    VkFramebuffer           m_handle = VK_NULL_HANDLE;
    uint32_t                m_attachmentCount = 0;
    uint32_t                m_width = 0;
    uint32_t                m_height = 0;
    uint32_t                m_layers = 0;

}; // class VulkanFramebuffer

#endif // VULKAN_FRAMEBUFFER_H