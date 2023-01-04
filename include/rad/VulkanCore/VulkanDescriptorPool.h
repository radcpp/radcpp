#ifndef VULKAN_DESCRIPTOR_POOL_H
#define VULKAN_DESCRIPTOR_POOL_H
#pragma once

#include "VulkanCommon.h"

class VulkanDescriptorPool : public VulkanObject
{
public:
    VulkanDescriptorPool(Ref<VulkanDevice> device, const VkDescriptorPoolCreateInfo& createInfo);
    ~VulkanDescriptorPool();

    VkDescriptorPool GetHandle() const { return m_handle; }

    Ref<VulkanDescriptorSet> Allocate(VulkanDescriptorSetLayout* layout);
    void Reset();

private:
    Ref<VulkanDevice>   m_device;
    VkDescriptorPool    m_handle = VK_NULL_HANDLE;

}; // class VulkanDescriptorPool

#endif // VULKAN_DESCRIPTOR_POOL_H