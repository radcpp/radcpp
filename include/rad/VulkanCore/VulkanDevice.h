#ifndef VULKAN_DEVICE_H
#define VULKAN_DEVICE_H
#pragma once

#include "VulkanCommon.h"

class VulkanDevice : public VulkanObject
{
public:
    VulkanDevice(Ref<VulkanInstance> instance, Ref<VulkanPhysicalDevice> physicalDevice, ArrayRef<std::string> extensionNames);
    ~VulkanDevice();

    VkDevice GetHandle() const { return m_handle; }
    const VolkDeviceTable* GetFunctionTable() const { return &m_functionTable; }
    VmaAllocator GetAllocator() const { return m_allocator; }

    VulkanPhysicalDevice* GetPhysicalDevice() const { return m_physicalDevice.get(); }
    const VkPhysicalDeviceLimits& GetLimits() const;

    bool SupportsExtension(std::string_view extension);

    uint32_t GetQueueFamilyIndex(VulkanQueueFamily queueFamily);
    VulkanQueue* GetQueue(VulkanQueueFamily queueFamily = VulkanQueueFamilyUniversal);
    bool SupportsSurface(VulkanQueueFamily queueFamily, VkSurfaceKHR surface) const;

    Ref<VulkanCommandPool> CreateCommandPool(
        VulkanQueueFamily queueFamily = VulkanQueueFamilyUniversal,
        VkCommandPoolCreateFlags flags = 0);
    // Allocate a command buffer from the internal CommandPool created with VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
    Ref<VulkanCommandBuffer> AllocateCommandBufferOneTimeUse(VulkanQueueFamily queueFamily = VulkanQueueFamilyUniversal);

    // Synchronization and Cache Control
    Ref<VulkanFence> CreateFence(VkFenceCreateFlags flags = 0);
    Ref<VulkanSemaphore> CreateSemaphore(VkSemaphoreCreateFlags flags = 0);
    Ref<VulkanSemaphore> CreateSemaphoreSignaled();
    Ref<VulkanSemaphore> CreateSemaphoreFromHandle(VkSemaphore semaphoreHandle);
    Ref<VulkanEvent> CreateEvent();
    void WaitIdle();

    // RenderPass
    Ref<VulkanRenderPass> CreateRenderPass(const VkRenderPassCreateInfo& createInfo);
    Ref<VulkanFramebuffer> CreateFramebuffer(
        VulkanRenderPass* renderPass,
        ArrayRef<VulkanImageView*> attachments,
        uint32_t width,
        uint32_t height,
        uint32_t layers
    );

    // Piplines
    Ref<VulkanShader> CreateShader(
        VkShaderStageFlagBits       stage,
        std::string_view            fileName,
        std::string_view            source,
        std::string_view            entryPoint,
        ArrayRef<ShaderMacro>       macros);
    Ref<VulkanShader> CreateShaderFromFile(
        VkShaderStageFlagBits       stage,
        std::string_view            fileName,
        std::string_view            entryPoint,
        ArrayRef<ShaderMacro>       macros);
    Ref<VulkanShaderModule> CreateShaderModule(ArrayRef<uint32_t> code);
    Ref<VulkanGraphicsPipeline> CreateGraphicsPipeline(const VulkanGraphicsPipelineCreateInfo& createInfo);
    Ref<VulkanComputePipeline> CreateComputePipeline(const VulkanComputePipelineCreateInfo& createInfo);

    // Resource Creation
    Ref<VulkanBuffer> CreateBuffer(const VulkanBufferCreateInfo& createInfo);
    Ref<VulkanBuffer> CreateUniformBuffer(VkDeviceSize size, bool isPersistentMapped = false);
    Ref<VulkanBuffer> CreateStorageBuffer(VkDeviceSize size, VkBufferUsageFlags additionalUsage = 0);
    Ref<VulkanBuffer> CreateIndexBuffer(VkDeviceSize size, VkBufferUsageFlags additionalUsage = 0);
    Ref<VulkanBuffer> CreateVertexBuffer(VkDeviceSize size, VkBufferUsageFlags additionalUsage = 0);
    Ref<VulkanBuffer> CreateStagingBuffer(VkDeviceSize size);
    Ref<VulkanImage> CreateImage(const VulkanImageCreateInfo& createInfo);
    Ref<VulkanImage> CreateImageFromHandle(VkImage imageHandle, const VkImageCreateInfo& createInfo);
    Ref<VulkanImage> CreateImage2D_DepthStencilAttachment(VkFormat format, uint32_t width, uint32_t height);

    // Samplers
    Ref<VulkanSampler> CreatSampler(const VkSamplerCreateInfo& createInfo);

    // Resource Descriptors
    Ref<VulkanDescriptorSetLayout> CreateDescriptorSetLayout(ArrayRef<VkDescriptorSetLayoutBinding> layoutBindings);
    Ref<VulkanPipelineLayout> CreatePipelineLayout(
        ArrayRef<VulkanDescriptorSetLayout*> descSetLayouts,
        ArrayRef<VkPushConstantRange> pushConstantRanges = {});
    Ref<VulkanDescriptorPool> CreateDescriptorPool(uint32_t maxSets, ArrayRef<VkDescriptorPoolSize> poolSizes);

    Ref<VulkanSwapchain> CreateSwapchain(const VkSwapchainCreateInfoKHR& createInfo);

private:
    Ref<VulkanInstance> m_instance;
    Ref<VulkanPhysicalDevice> m_physicalDevice;
    VkDevice m_handle = VK_NULL_HANDLE;
    // function pointers
    VolkDeviceTable m_functionTable;
    // manage device memory allocations
    VmaAllocator m_allocator = nullptr;

    int m_queueFamilyIndices[VulkanQueueFamilyCount];
    Ref<VulkanQueue> m_queues[VulkanQueueFamilyCount];
    Ref<VulkanCommandPool> m_commandPoolsTransientAlloc[VulkanQueueFamilyCount];

    std::vector<std::string> m_enabledExtensionNames;

}; // class VulkanDevice

#endif // VULKAN_DEVICE_H