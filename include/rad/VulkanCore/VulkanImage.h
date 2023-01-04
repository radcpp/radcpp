#ifndef VULKAN_IMAGE_H
#define VULKAN_IMAGE_H
#pragma once

#include "VulkanCommon.h"
#include "rad/Core/File.h"

class VulkanImage : public VulkanObject
{
public:
    VulkanImage(Ref<VulkanDevice> device, const VulkanImageCreateInfo& createInfo);
    VulkanImage(Ref<VulkanDevice> device, VkImage handle, const VkImageCreateInfo& createInfo);
    ~VulkanImage();

    static Ref<VulkanImage> CreateImage2DFromFile(VulkanDevice* device, const Path& filePath, bool bGenerateMipmaps);

    VkImage GetHandle() const { return m_handle; }
    VkImageType GetType() const { return m_type; }
    VkFormat GetFormat() const { return m_format; }
    VkExtent3D GetExtent() const { return m_extent; }
    uint32_t GetWidth() const { return m_extent.width; }
    uint32_t GetHeight() const { return m_extent.height; }
    uint32_t GetDepth() const { return m_extent.depth; }
    uint32_t GetMipLevels() const { return m_mipLevels; }
    uint32_t GetArrayLayers() const { return m_arrayLayers; }
    VkSampleCountFlagBits GetSampleCount() const { return m_samples; }
    VkImageUsageFlags GetUsage() const { return m_usage; }

    /**
    If componentMapping is nullptr, use default component mapping:
        components.r = VK_COMPONENT_SWIZZLE_R;
        components.g = VK_COMPONENT_SWIZZLE_G;
        components.b = VK_COMPONENT_SWIZZLE_B;
        components.a = VK_COMPONENT_SWIZZLE_A;
    */
    Ref<VulkanImageView> CreateImageView(
        VkImageViewType                     type,
        VkFormat                            format,
        const VkImageSubresourceRange&      subresourceRange,
        const VkComponentMapping*           componentMapping = nullptr);
    Ref<VulkanImageView> CreateImageView2D(uint32_t baseMipLevel = 0, uint32_t levelCount = 1, uint32_t baseArrayLayer = 0);

    VulkanImageView* GetDefaultView() { return m_defaultView.get(); }

    void CopyFromBuffer(VulkanBuffer* buffer, ArrayRef<VkBufferImageCopy> copyInfos);
    void CopyFromBuffer2D(
        VulkanBuffer* buffer, VkDeviceSize bufferOffset,
        uint32_t baseMipLevel, uint32_t levelCount,
        uint32_t baseArrayLayer, uint32_t layerCount);
    void Write2D(
        VulkanBuffer* buffer, VkDeviceSize bufferOffset,
        uint32_t baseMipLevel, uint32_t levelCount,
        uint32_t baseArrayLayer, uint32_t layerCount);

private:
    Ref<VulkanImageView> CreateDefaultView();

    Ref<VulkanDevice>           m_device;
    VkImage                     m_handle = VK_NULL_HANDLE;
    VkImageCreateFlags          m_createFlags;
    VkImageType                 m_type;
    VkFormat                    m_format;
    VkExtent3D                  m_extent;
    uint32_t                    m_mipLevels;
    uint32_t                    m_arrayLayers;
    VkSampleCountFlagBits       m_samples;
    VkImageTiling               m_tiling;
    VkImageUsageFlags           m_usage;
    VkSharingMode               m_sharingMode;

    // Memory management
    VmaAllocation               m_allocation;
    VmaAllocationInfo           m_allocationInfo;
    VkMemoryPropertyFlags       m_memoryFlags;

    Ref<VulkanImageView>        m_defaultView;

}; // class VulkanImage

class VulkanImageView : public VulkanObject
{
public:
    VulkanImageView(Ref<VulkanDevice> device, Ref<VulkanImage> image, const VkImageViewCreateInfo& createInfo);
    ~VulkanImageView();

    VkImageView GetHandle() const { return m_handle; }

private:
    Ref<VulkanDevice>       m_device;
    Ref<VulkanImage>        m_image;
    VkImageView             m_handle = VK_NULL_HANDLE;

}; // class VulkanImageView

#endif // VULKAN_IMAGE_H