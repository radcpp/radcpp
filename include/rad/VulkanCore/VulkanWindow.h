#ifndef VULKAN_WINDOW_H
#define VULKAN_WINDOW_H
#pragma once

#include "rad/Gui/Application.h"
#include "rad/Gui/Window.h"
#include "VulkanCommon.h"

class VulkanWindow : public Window
{
public:
    VulkanWindow(Ref<VulkanInstance> instance);
    virtual ~VulkanWindow();

    virtual bool Create(const char* title, int x, int y, int width, int height, Uint32 flags);
    virtual void Destroy();

    bool Init(Ref<VulkanDevice> device);

    void GetDrawableSize(int* w, int* h);

    VulkanCommandBuffer* BeginFrame();
    void EndFrame();

    // Get the default command buffer corresponding to current swapchainImage
    VulkanCommandBuffer* GetCommandBuffer();

    VulkanInstance* GetInstance() { return m_instance.get(); }
    VulkanDevice* GetDevice() { return m_device.get(); }
    VulkanSwapchain* GetSwapchain() { return m_swapchain.get(); }
    VulkanRenderPass* GetDefaultRenderPass() { return m_defaultRenderPass.get(); }
    VulkanImage* GetDefaultDepthStencilImage() { return m_depthStencil.get(); }

protected:
    virtual void OnResized(int width, int height);
    virtual void OnClose();

    bool CreateSizeDependentResources(uint32_t width, uint32_t height);
    bool CreateSwapchain(uint32_t width, uint32_t height);
    bool CreateDepthStencil(uint32_t width, uint32_t height);
    bool CreateDefaultRenderPass();
    bool CreateDefaultFramebuffers(uint32_t width, uint32_t height);

    Ref<VulkanInstance> m_instance;
    Ref<VulkanDevice> m_device;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSurfaceCapabilitiesKHR m_surfaceCapabilities = {};

    Ref<VulkanCommandPool> m_commandPool;
    std::vector<Ref<VulkanCommandBuffer>> m_commandBuffers;

    Ref<VulkanSwapchain> m_swapchain;
    VkSurfaceFormatKHR m_surfaceFormat;
    uint32_t m_swapchainImageCount = 3;
    bool m_enableVSync = false;
    uint32_t m_swapchainImageIndex = 0;

    VkFormat m_depthStencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    Ref<VulkanImage> m_depthStencil;

    Ref<VulkanRenderPass> m_defaultRenderPass;
    std::vector<Ref<VulkanFramebuffer>> m_defaultFramebuffers;

    // max frame count in flight
    static constexpr uint32_t FrameLag = 2;
    Ref<VulkanSemaphore> m_swapchainImageAcquired[FrameLag];
    Ref<VulkanSemaphore> m_drawComplete[FrameLag];
    Ref<VulkanSemaphore> m_swapchainImageOwnershipTransferComplete[FrameLag];
    // Fences that we can use to throttle if we get too far ahead of image presents.
    Ref<VulkanFence> m_frameThrottles[FrameLag];
    uint32_t m_frameIndex = 0;

}; // class VulkanWindow

#endif // VULKAN_WINDOW_H