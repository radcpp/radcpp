#include "rad/VulkanCore/VulkanImage.h"
#include "rad/VulkanCore/VulkanDevice.h"
#include "rad/VulkanCore/VulkanPhysicalDevice.h"
#include "rad/VulkanCore/VulkanQueue.h"
#include "rad/VulkanCore/VulkanCommandBuffer.h"
#include "rad/VulkanCore/VulkanBuffer.h"

#include "vk_format_utils.h"

#include "compressonator/compressonator.h"

VulkanImage::VulkanImage(Ref<VulkanDevice> device, const VulkanImageCreateInfo& createInfo) :
    m_device(std::move(device))
{
    m_createFlags = createInfo.m_createInfo.flags;
    m_type = createInfo.m_createInfo.imageType;
    m_format = createInfo.m_createInfo.format;
    m_extent = createInfo.m_createInfo.extent;
    m_mipLevels = createInfo.m_createInfo.mipLevels;
    m_arrayLayers = createInfo.m_createInfo.arrayLayers;
    m_samples = createInfo.m_createInfo.samples;
    m_tiling = createInfo.m_createInfo.tiling;
    m_usage = createInfo.m_createInfo.usage;
    m_sharingMode = createInfo.m_createInfo.sharingMode;

    VK_CHECK(vmaCreateImage(m_device->GetAllocator(), &createInfo.m_createInfo, &createInfo.m_allocation,
        &m_handle, &m_allocation, &m_allocationInfo));

    if (m_handle != VK_NULL_HANDLE)
    {
        vmaGetMemoryTypeProperties(m_device->GetAllocator(), m_allocationInfo.memoryType, &m_memoryFlags);
    }

    m_defaultView = CreateDefaultView();
}

VulkanImage::VulkanImage(Ref<VulkanDevice> device, VkImage handle, const VkImageCreateInfo& createInfo) :
    m_device(std::move(device)),
    m_handle(handle)
{
    m_type = VK_IMAGE_TYPE_2D;
    m_format = createInfo.format;
    m_extent.width = createInfo.extent.width;
    m_extent.height = createInfo.extent.height;
    m_extent.depth = 1;
    m_mipLevels = 1;
    m_arrayLayers = createInfo.arrayLayers;
    m_samples = VK_SAMPLE_COUNT_1_BIT;
    m_tiling = VK_IMAGE_TILING_OPTIMAL;
    m_usage = createInfo.usage;
    m_sharingMode = createInfo.sharingMode;

    m_allocation = nullptr;
    m_memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_defaultView = CreateDefaultView();
}

VulkanImage::~VulkanImage()
{
    // If there is no allocation, the image is swapchain image and the resource is not managed.
    if (m_handle && m_allocation)
    {
        vmaDestroyImage(m_device->GetAllocator(), m_handle, m_allocation);
        m_handle = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
    }
}

Ref<VulkanImageView> VulkanImage::CreateImageView(
    VkImageViewType                     type,
    VkFormat                            format,
    const VkImageSubresourceRange&      subresourceRange,
    const VkComponentMapping*           componentMapping)
{
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0; // reserved for future use
    createInfo.image = m_handle;
    createInfo.viewType = type;
    createInfo.format = format;
    if (componentMapping)
    {
        createInfo.components = *componentMapping;
    }
    else
    {
        createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    }
    createInfo.subresourceRange = subresourceRange;

    return new VulkanImageView(m_device, this, createInfo);
}

Ref<VulkanImageView> VulkanImage::CreateImageView2D(uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer)
{
    VkImageSubresourceRange range = {};
    range.aspectMask = VulkanFormat(m_format).GetAspectFlags();
    range.baseMipLevel = baseMipLevel;
    range.levelCount = levelCount;
    range.baseArrayLayer = baseArrayLayer;
    range.layerCount = 1;
    return CreateImageView(VK_IMAGE_VIEW_TYPE_2D, m_format, range);
}

void VulkanImage::CopyFromBuffer(VulkanBuffer* buffer, ArrayRef<VkBufferImageCopy> copyInfos)
{
    Ref<VulkanCommandBuffer> commandBuffer = m_device->AllocateCommandBufferOneTimeUse();
    commandBuffer->Begin();
    commandBuffer->TransitLayout(this,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    commandBuffer->CopyBufferToImage(buffer, this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copyInfos);
    commandBuffer->TransitLayout(this,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    commandBuffer->End();

    m_device->GetQueue()->SubmitAndWaitForCompletion({ commandBuffer.get() });
}

void VulkanImage::CopyFromBuffer2D(VulkanBuffer* buffer, VkDeviceSize bufferOffset, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount)
{
    std::vector<VkBufferImageCopy> copyInfos(levelCount);
    VkExtent3D blockExtent = FormatTexelBlockExtent(m_format);
    uint32_t blockSize = FormatElementSize(m_format);
    for (uint32_t mipLevel = baseMipLevel; mipLevel < levelCount; mipLevel++)
    {
        uint32_t mipWidth = std::max<uint32_t>(m_extent.width >> mipLevel, 1);
        uint32_t mipHeight = std::max<uint32_t>(m_extent.height >> mipLevel, 1);

        copyInfos[mipLevel].bufferOffset = bufferOffset;
        copyInfos[mipLevel].bufferRowLength = RoundUpToMultiple(mipWidth, blockExtent.width);
        copyInfos[mipLevel].bufferImageHeight = RoundUpToMultiple(mipHeight, blockExtent.height);
        copyInfos[mipLevel].imageSubresource.aspectMask = VulkanFormat(m_format).GetAspectFlags();
        copyInfos[mipLevel].imageSubresource.mipLevel = mipLevel;
        copyInfos[mipLevel].imageSubresource.baseArrayLayer = baseArrayLayer;
        copyInfos[mipLevel].imageSubresource.layerCount = layerCount;
        copyInfos[mipLevel].imageOffset = {};
        copyInfos[mipLevel].imageExtent.width = mipWidth;
        copyInfos[mipLevel].imageExtent.height = mipHeight;
        copyInfos[mipLevel].imageExtent.depth = 1;

        bufferOffset += (copyInfos[mipLevel].bufferRowLength / blockExtent.width) * (copyInfos[mipLevel].bufferImageHeight / blockExtent.height) * blockSize * layerCount;
    }
    CopyFromBuffer(buffer, copyInfos);
}

void VulkanImage::Write2D(VulkanBuffer* buffer, VkDeviceSize bufferOffset, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount)
{
    CopyFromBuffer2D(buffer, bufferOffset, baseMipLevel, levelCount, baseArrayLayer, layerCount);
}

Ref<VulkanImageView> VulkanImage::CreateDefaultView()
{
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_1D;
    if (m_type == VkImageType::VK_IMAGE_TYPE_1D)
    {
        viewType = VK_IMAGE_VIEW_TYPE_1D;
        if (m_arrayLayers > 1)
        {
            viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        }
    }
    else if (m_type == VkImageType::VK_IMAGE_TYPE_2D)
    {
        viewType = VK_IMAGE_VIEW_TYPE_2D;
        if (m_arrayLayers > 1)
        {
            viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            if (m_createFlags | VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
            {
                assert(m_arrayLayers % 6 == 0);
                if (m_arrayLayers == 6)
                {
                    viewType = VK_IMAGE_VIEW_TYPE_CUBE;
                }
                else
                {
                    viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                }
            }
        }
    }
    else if (m_type == VkImageType::VK_IMAGE_TYPE_3D)
    {
        viewType = VK_IMAGE_VIEW_TYPE_3D;
    }

    VkImageSubresourceRange wholeRange = {};
    wholeRange.aspectMask = VulkanFormat(m_format).GetAspectFlags();
    wholeRange.baseMipLevel = 0;
    wholeRange.levelCount = m_mipLevels;
    wholeRange.baseArrayLayer = 0;
    wholeRange.layerCount = m_arrayLayers;

    return CreateImageView(viewType, m_format, wholeRange);
}

VulkanImageView::VulkanImageView(Ref<VulkanDevice> device, Ref<VulkanImage> image, const VkImageViewCreateInfo& createInfo) :
    m_device(std::move(device)),
    m_image(std::move(image))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateImageView(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

VulkanImageView::~VulkanImageView()
{
    m_device->GetFunctionTable()->
        vkDestroyImageView(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}

VkFormat MapCMPFormatToVulkanFormat(CMP_FORMAT srcFormat)
{
    switch (srcFormat)
    {
        // Channel Component formats --------------------------------------------------------------------------------
    case CMP_FORMAT_RGBA_8888_S: return VK_FORMAT_R8G8B8A8_SNORM; // RGBA format with signed 8-bit fixed channels.
    case CMP_FORMAT_ARGB_8888_S: return VK_FORMAT_UNDEFINED; // ARGB format with signed 8-bit fixed channels.
    case CMP_FORMAT_ARGB_8888:   return VK_FORMAT_UNDEFINED; // ARGB format with 8-bit fixed channels.
    case CMP_FORMAT_ABGR_8888:   return VK_FORMAT_UNDEFINED; // ABGR format with 8-bit fixed channels.
    case CMP_FORMAT_RGBA_8888:   return VK_FORMAT_R8G8B8A8_UNORM; // RGBA format with 8-bit fixed channels.
    case CMP_FORMAT_BGRA_8888:   return VK_FORMAT_B8G8R8A8_UNORM; // BGRA format with 8-bit fixed channels.
    case CMP_FORMAT_RGB_888:     return VK_FORMAT_UNDEFINED; // RGB format with 8-bit fixed channels.
    case CMP_FORMAT_RGB_888_S:   return VK_FORMAT_UNDEFINED; // RGB format with 8-bit fixed channels.
    case CMP_FORMAT_BGR_888:     return VK_FORMAT_UNDEFINED; // BGR format with 8-bit fixed channels.
    case CMP_FORMAT_RG_8_S:      return VK_FORMAT_UNDEFINED; // Two component format with signed 8-bit fixed channels.
    case CMP_FORMAT_RG_8:        return VK_FORMAT_UNDEFINED; // Two component format with 8-bit fixed channels.
    case CMP_FORMAT_R_8_S:       return VK_FORMAT_UNDEFINED; // Single component format with signed 8-bit fixed channel.
    case CMP_FORMAT_R_8:         return VK_FORMAT_R8_UNORM; // Single component format with 8-bit fixed channel.
    case CMP_FORMAT_ARGB_2101010:return VK_FORMAT_UNDEFINED; // ARGB format with 10-bit fixed channels for color & a 2-bit fixed channel for alpha.
    case CMP_FORMAT_ARGB_16:     return VK_FORMAT_UNDEFINED; // ARGB format with 16-bit fixed channels.
    case CMP_FORMAT_ABGR_16:     return VK_FORMAT_UNDEFINED; // ABGR format with 16-bit fixed channels.
    case CMP_FORMAT_RGBA_16:     return VK_FORMAT_R16G16B16A16_UNORM; // RGBA format with 16-bit fixed channels.
    case CMP_FORMAT_BGRA_16:     return VK_FORMAT_UNDEFINED; // BGRA format with 16-bit fixed channels.
    case CMP_FORMAT_RG_16:       return VK_FORMAT_R16G16_UNORM; // Two component format with 16-bit fixed channels.
    case CMP_FORMAT_R_16:        return VK_FORMAT_R16_UNORM; // Single component format with 16-bit fixed channels.
    case CMP_FORMAT_RGBE_32F:    return VK_FORMAT_UNDEFINED; // RGB format with 9-bit floating point each channel and shared 5 bit exponent
    case CMP_FORMAT_ARGB_16F:    return VK_FORMAT_UNDEFINED; // ARGB format with 16-bit floating-point channels.
    case CMP_FORMAT_ABGR_16F:    return VK_FORMAT_UNDEFINED; // ABGR format with 16-bit floating-point channels.
    case CMP_FORMAT_RGBA_16F:    return VK_FORMAT_R16G16B16A16_SFLOAT; // RGBA format with 16-bit floating-point channels.
    case CMP_FORMAT_BGRA_16F:    return VK_FORMAT_UNDEFINED; // BGRA format with 16-bit floating-point channels.
    case CMP_FORMAT_RG_16F:      return VK_FORMAT_R16G16_SFLOAT; // Two component format with 16-bit floating-point channels.
    case CMP_FORMAT_R_16F:       return VK_FORMAT_R16_SFLOAT; // Single component with 16-bit floating-point channels.
    case CMP_FORMAT_ARGB_32F:    return VK_FORMAT_UNDEFINED; // ARGB format with 32-bit floating-point channels.
    case CMP_FORMAT_ABGR_32F:    return VK_FORMAT_UNDEFINED; // ABGR format with 32-bit floating-point channels.
    case CMP_FORMAT_RGBA_32F:    return VK_FORMAT_R32G32B32A32_SFLOAT; // RGBA format with 32-bit floating-point channels.
    case CMP_FORMAT_BGRA_32F:    return VK_FORMAT_UNDEFINED; // BGRA format with 32-bit floating-point channels.
    case CMP_FORMAT_RGB_32F:     return VK_FORMAT_UNDEFINED; // RGB format with 32-bit floating-point channels.
    case CMP_FORMAT_BGR_32F:     return VK_FORMAT_UNDEFINED; // BGR format with 32-bit floating-point channels.
    case CMP_FORMAT_RG_32F:      return VK_FORMAT_R32G32_SFLOAT; // Two component format with 32-bit floating-point channels.
    case CMP_FORMAT_R_32F:       return VK_FORMAT_R32_SFLOAT; // Single component with 32-bit floating-point channels.
        // Compression formats ------------ GPU Mapping DirectX, Vulkan and OpenGL formats and comments --------
    case CMP_FORMAT_ASTC:           return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    case CMP_FORMAT_ATI1N:          return VK_FORMAT_BC4_UNORM_BLOCK;
    case CMP_FORMAT_ATI2N:          return VK_FORMAT_BC5_UNORM_BLOCK;
    case CMP_FORMAT_ATI2N_XY:       return VK_FORMAT_BC5_UNORM_BLOCK;
    case CMP_FORMAT_ATI2N_DXT5:     return VK_FORMAT_BC5_UNORM_BLOCK;
    case CMP_FORMAT_ATC_RGB:                return VK_FORMAT_UNDEFINED; // CMP - a compressed RGB format.
    case CMP_FORMAT_ATC_RGBA_Explicit:      return VK_FORMAT_UNDEFINED; // CMP - a compressed ARGB format with explicit alpha.
    case CMP_FORMAT_ATC_RGBA_Interpolated:  return VK_FORMAT_UNDEFINED; // CMP - a compressed ARGB format with interpolated alpha.
    case CMP_FORMAT_BC1:            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case CMP_FORMAT_BC2:            return VK_FORMAT_BC2_UNORM_BLOCK;
    case CMP_FORMAT_BC3:            return VK_FORMAT_BC3_UNORM_BLOCK;
    case CMP_FORMAT_BC4:            return VK_FORMAT_BC4_UNORM_BLOCK;
    case CMP_FORMAT_BC4_S:          return VK_FORMAT_BC4_SNORM_BLOCK;
    case CMP_FORMAT_BC5:            return VK_FORMAT_BC5_UNORM_BLOCK;
    case CMP_FORMAT_BC5_S:          return VK_FORMAT_BC5_SNORM_BLOCK;
    case CMP_FORMAT_BC6H:           return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case CMP_FORMAT_BC6H_SF:        return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case CMP_FORMAT_BC7:            return VK_FORMAT_BC7_UNORM_BLOCK;
    case CMP_FORMAT_DXT1:           return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case CMP_FORMAT_DXT3:           return VK_FORMAT_BC2_UNORM_BLOCK;
    case CMP_FORMAT_DXT5:           return VK_FORMAT_BC3_UNORM_BLOCK;
    case CMP_FORMAT_DXT5_xGBR:      return VK_FORMAT_UNDEFINED;// DXGI_FORMAT_UNKNOWN DXT5 with the red component swizzled into the alpha channel. Eight bits per pixel.
    case CMP_FORMAT_DXT5_RxBG:      return VK_FORMAT_UNDEFINED;// DXGI_FORMAT_UNKNOWN swizzled DXT5 format with the green component swizzled into the alpha channel. Eight bits per pixel.
    case CMP_FORMAT_DXT5_RBxG:      return VK_FORMAT_UNDEFINED;// DXGI_FORMAT_UNKNOWN swizzled DXT5 format with the green component swizzled into the alpha channel & the blue component swizzled into the green channel. Eight bits per pixel.
    case CMP_FORMAT_DXT5_xRBG:      return VK_FORMAT_UNDEFINED;// DXGI_FORMAT_UNKNOWN swizzled DXT5 format with the green component swizzled into the alpha channel & the red component swizzled into the green channel. Eight bits per pixel.
    case CMP_FORMAT_DXT5_RGxB:      return VK_FORMAT_UNDEFINED;// DXGI_FORMAT_UNKNOWN swizzled DXT5 format with the blue component swizzled into the alpha channel. Eight bits per pixel.
    case CMP_FORMAT_DXT5_xGxR:      return VK_FORMAT_UNDEFINED;// two-component swizzled DXT5 format with the red component swizzled into the alpha channel & the green component in the green channel. Eight bits per pixel.
    case CMP_FORMAT_ETC_RGB:        return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    case CMP_FORMAT_ETC2_RGB:       return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    case CMP_FORMAT_ETC2_SRGB:      return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
    case CMP_FORMAT_ETC2_RGBA:      return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    case CMP_FORMAT_ETC2_RGBA1:     return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
    case CMP_FORMAT_ETC2_SRGBA:     return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
    case CMP_FORMAT_ETC2_SRGBA1:    return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
    }

    return VK_FORMAT_UNDEFINED;
}

Ref<VulkanImage> VulkanImage::CreateImage2DFromFile(VulkanDevice* device, const Path& filePath, bool bGenerateMipmaps)
{
    Ref<VulkanImage> image;

    // RAII wrapper for CMP_MipSet
    class MipSet
    {
        CMP_MipSet m_mipSet = {};
    public:
        MipSet()
        {
        }
        ~MipSet()
        {
            CMP_FreeMipSet(&m_mipSet);
        }
        operator CMP_MipSet* ()
        {
            return &m_mipSet;
        }
        CMP_MipSet* operator->()
        {
            return &m_mipSet;
        }
        CMP_MipSet* operator&()
        {
            return &m_mipSet;
        }
    } mipSet;

    std::string fileName = (const char*)filePath.u8string().c_str();
    CMP_ERROR status = CMP_LoadTexture(fileName.c_str(), &mipSet);
    if (status != CMP_OK)
    {
        return nullptr;
    }

    if (bGenerateMipmaps && (mipSet->m_nMipLevels <= 1))
    {
        CMP_INT maxLevel = static_cast<CMP_INT>(GetMaxMipLevel(mipSet->m_nHeight, mipSet->m_nWidth, 1));
        CMP_INT minMipSize = CMP_CalcMinMipSize(mipSet->m_nHeight, mipSet->m_nWidth, maxLevel);
        CMP_GenerateMIPLevels(mipSet, minMipSize);
    }

    VulkanImageCreateInfo createInfo = {};
    VkFormat format = MapCMPFormatToVulkanFormat(mipSet->m_format);
    createInfo.SetTexture2D(format,
        mipSet->m_nWidth, mipSet->m_nHeight);
    createInfo.m_createInfo.mipLevels = mipSet->m_nMipLevels;

    if (VulkanFormat(format).IsCompressed())
    {
        image = device->CreateImage(createInfo);
        Ref<VulkanBuffer> stagingBuffer = device->CreateStagingBuffer(mipSet->dwDataSize);
        stagingBuffer->Write(mipSet->pData);
        image->Write2D(stagingBuffer.get(), 0,
            0, mipSet->m_nMipLevels, 0, 1);
    }
    else
    {
        image = device->CreateImage(createInfo);
        VkDeviceSize dataSize = 0;
        for (uint32_t level = 0; level < static_cast<uint32_t>(mipSet->m_nMipLevels); ++level)
        {
            dataSize += mipSet->m_pMipLevelTable[level]->m_dwLinearSize;
        }
        Ref<VulkanBuffer> stagingBuffer = device->CreateStagingBuffer(dataSize);
        uint8_t* pStaging = (uint8_t*)stagingBuffer->MapMemory(0, stagingBuffer->GetSize());
        for (uint32_t level = 0; level < static_cast<uint32_t>(mipSet->m_nMipLevels); ++level)
        {
            uint32_t mipDataSize = mipSet->m_pMipLevelTable[level]->m_dwLinearSize;
            memcpy(pStaging, mipSet->m_pMipLevelTable[level]->m_pbData, mipDataSize);
            pStaging += mipDataSize;
        }
        stagingBuffer->UnmapMemory();

        image->Write2D(stagingBuffer.get(), 0,
            0, mipSet->m_nMipLevels, 0, 1);
    }

    return image;
}
