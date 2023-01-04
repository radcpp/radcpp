#include "rad/VulkanCore/VulkanSwapchain.h"
#include "rad/VulkanCore/VulkanDevice.h"
#include "rad/VulkanCore/VulkanImage.h"
#include "rad/VulkanCore/VulkanFence.h"
#include "rad/VulkanCore/VulkanSemaphore.h"

VulkanSwapchain::VulkanSwapchain(Ref<VulkanDevice> device, const VkSwapchainCreateInfoKHR& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateSwapchainKHR(m_device->GetHandle(), &createInfo, nullptr, &m_handle));

    m_imageCount = createInfo.minImageCount;
    m_format = createInfo.imageFormat;
    m_width = createInfo.imageExtent.width;
    m_height = createInfo.imageExtent.height;

    std::vector<VkImage> m_imageHandles(m_imageCount);
    VK_CHECK(m_device->GetFunctionTable()->
        vkGetSwapchainImagesKHR(m_device->GetHandle(), m_handle, &m_imageCount, nullptr));
    if (m_imageCount > 0)
    {
        VK_CHECK(m_device->GetFunctionTable()->
            vkGetSwapchainImagesKHR(m_device->GetHandle(), m_handle, &m_imageCount, m_imageHandles.data()));
    }

    m_images.resize(m_imageCount);
    for (uint32_t i = 0; i < m_imageCount; i++)
    {
        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.pNext = nullptr;
        imageCreateInfo.flags = 0;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = createInfo.imageFormat;
        imageCreateInfo.extent.width = createInfo.imageExtent.width;
        imageCreateInfo.extent.height = createInfo.imageExtent.height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = createInfo.imageArrayLayers;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = createInfo.imageUsage;
        imageCreateInfo.sharingMode = createInfo.imageSharingMode;
        imageCreateInfo.queueFamilyIndexCount = createInfo.queueFamilyIndexCount;
        imageCreateInfo.pQueueFamilyIndices = createInfo.pQueueFamilyIndices;
        imageCreateInfo.initialLayout;
        m_images[i] = new VulkanImage(m_device, m_imageHandles[i], imageCreateInfo);
    }
}

VulkanSwapchain::~VulkanSwapchain()
{
    m_device->GetFunctionTable()->
        vkDestroySwapchainKHR(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}

uint32_t VulkanSwapchain::GetImageCount()
{
    return m_imageCount;
}

VkFormat VulkanSwapchain::GetFormat()
{
    return m_format;
}

uint32_t VulkanSwapchain::GetWidth()
{
    return m_width;
}

uint32_t VulkanSwapchain::GetHeight()
{
    return m_height;
}

VulkanImage* VulkanSwapchain::GetImage(uint32_t index)
{
    return m_images[index].get();
}

VulkanImageView* VulkanSwapchain::GetDefaultView(uint32_t index)
{
    return m_images[index]->GetDefaultView();
}

VulkanImage* VulkanSwapchain::GetCurrentImage()
{
    return m_images[m_currentImageIndex].get();
}

uint32_t VulkanSwapchain::GetCurrentImageIndex() const
{
    return m_currentImageIndex;
}

uint32_t VulkanSwapchain::AcquireNextImage(VulkanSemaphore* semaphore, VulkanFence* fence, uint64_t timeout)
{
    VkSemaphore semaphoreHandle = semaphore ? semaphore->GetHandle() : VK_NULL_HANDLE;
    VkFence fenceHandle = fence ? fence->GetHandle() : VK_NULL_HANDLE;
    VK_CHECK(m_device->GetFunctionTable()->
        vkAcquireNextImageKHR(m_device->GetHandle(), m_handle, timeout, semaphoreHandle, fenceHandle, &m_currentImageIndex));
    return m_currentImageIndex;
}
