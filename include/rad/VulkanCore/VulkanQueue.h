#ifndef VULKAN_QUEUE_H
#define VULKAN_QUEUE_H
#pragma once

#include "VulkanCommon.h"

using VulkanSubmitWait = std::pair<VulkanSemaphore*, VkPipelineStageFlags>;

class VulkanQueue : public VulkanObject
{
public:
    VulkanQueue(Ref<VulkanDevice> device, VulkanQueueFamily queueFamily);
    ~VulkanQueue();

    VkQueue GetHandle() const { return m_handle; }

    VulkanQueueFamily GetQueueFamily() const { return m_queueFamily; }
    const VkQueueFamilyProperties& GetQueueFamilyProperties() const;

    void Submit(
        ArrayRef<VulkanCommandBuffer*>  commandBuffers,
        ArrayRef<VulkanSubmitWait>      waitSemaphores,
        ArrayRef<VulkanSemaphore*>      signalSemaphores,
        VulkanFence*                    fence
    );

    // Create a fence implicitly; wait GPU to complete the commands and notify the host.
    void SubmitAndWaitForCompletion(
        ArrayRef<VulkanCommandBuffer*>  commandBuffers,
        ArrayRef<VulkanSubmitWait>      waitSemaphores = {},
        ArrayRef<VulkanSemaphore*>      signalSemaphores = {}
    );

    void WaitIdle();

    bool Present(
        ArrayRef<VulkanSemaphore*>      waitSemaphores,
        ArrayRef<VulkanSwapchain*>      swapchains,
        ArrayRef<uint32_t>              imageIndices);
    bool Present(
        ArrayRef<VulkanSemaphore*>      waitSemaphores,
        ArrayRef<VulkanSwapchain*>      swapchains);

private:
    Ref<VulkanDevice>           m_device;
    VkQueue                     m_handle = VK_NULL_HANDLE;
    VulkanQueueFamily           m_queueFamily;

}; // class VulkanQueue

#endif // VULKAN_QUEUE_H
