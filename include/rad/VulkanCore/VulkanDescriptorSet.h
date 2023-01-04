#ifndef VULKAN_DESCRIPTOR_SET_H
#define VULKAN_DESCRIPTOR_SET_H
#pragma once

#include "VulkanCommon.h"

class VulkanDescriptorSetLayout : public VulkanObject
{
public:
    VulkanDescriptorSetLayout(Ref<VulkanDevice> device, const VkDescriptorSetLayoutCreateInfo& createInfo);
    ~VulkanDescriptorSetLayout();

    VkDescriptorSetLayout GetHandle() const { return m_handle; }

private:
    Ref<VulkanDevice>           m_device;
    VkDescriptorSetLayout       m_handle = VK_NULL_HANDLE;

}; // class VulkanDescriptorSetLayout

class VulkanDescriptorSet : public VulkanObject
{
public:
    VulkanDescriptorSet(Ref<VulkanDevice> device, Ref<VulkanDescriptorPool> descriptorPool, Ref<VulkanDescriptorSetLayout> layout);
    ~VulkanDescriptorSet();

    VkDescriptorSet GetHandle() const { return m_handle; }

    void Update(ArrayRef<VkWriteDescriptorSet> writes, ArrayRef<VkCopyDescriptorSet> copies = {});
    void UpdateBuffers(
        uint32_t                    binding,
        uint32_t                    arrayElement,
        VkDescriptorType            type,
        ArrayRef<VulkanBuffer*>     buffers);
    void UpdateImages(
        uint32_t                    binding,
        uint32_t                    arrayElement,
        VkDescriptorType            type,
        ArrayRef<VulkanImageView*>  imageViews,
        VkImageLayout               layout);

private:
    Ref<VulkanDevice>               m_device;
    Ref<VulkanDescriptorPool>       m_descriptorPool;
    Ref<VulkanDescriptorSetLayout>  m_layout;
    VkDescriptorSet                 m_handle = VK_NULL_HANDLE;

}; // class VulkanDescriptorSet

#endif // VULKAN_DESCRIPTOR_SET_H
