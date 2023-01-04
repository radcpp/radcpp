#include "rad/VulkanCore/VulkanDevice.h"
#include "rad/VulkanCore.h"

VulkanDevice::VulkanDevice(
    Ref<VulkanInstance> instance, Ref<VulkanPhysicalDevice> physicalDevice, ArrayRef<std::string> extensionNames) :
    m_instance(std::move(instance)),
    m_physicalDevice(std::move(physicalDevice))
{
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext;
    createInfo.flags;

    for (size_t i = 0; i < VulkanQueueFamilyCount; ++i)
    {
        m_queueFamilyIndices[i] = VK_QUEUE_FAMILY_IGNORED;
    }

    const std::vector<VkQueueFamilyProperties>& queueFamilyProps = m_physicalDevice->GetQueueFamilyProperties();
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    float queuePriorities[1] = { 1.0f };
    for (int i = 0; i < queueFamilyProps.size(); i++)
    {
        if ((m_queueFamilyIndices[VulkanQueueFamilyUniversal] == VK_QUEUE_FAMILY_IGNORED)
            && HasAnyBit(queueFamilyProps[i].queueFlags, VK_QUEUE_GRAPHICS_BIT))
        {
            m_queueFamilyIndices[VulkanQueueFamilyUniversal] = i;
            VkDeviceQueueCreateInfo queueInfo = {};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.pNext;
            queueInfo.flags;
            queueInfo.queueFamilyIndex = i;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = queuePriorities;
            queueInfos.push_back(queueInfo);
        }
        if ((m_queueFamilyIndices[VulkanQueueFamilyCompute] == VK_QUEUE_FAMILY_IGNORED)
            && HasAnyBit(queueFamilyProps[i].queueFlags, VK_QUEUE_COMPUTE_BIT)
            && HasNoBit(queueFamilyProps[i].queueFlags, VK_QUEUE_GRAPHICS_BIT))
        {
            m_queueFamilyIndices[VulkanQueueFamilyCompute] = i;
            VkDeviceQueueCreateInfo queueInfo = {};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.pNext;
            queueInfo.flags;
            queueInfo.queueFamilyIndex = i;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = queuePriorities;
            queueInfos.push_back(queueInfo);
        }
        if ((m_queueFamilyIndices[VulkanQueueFamilyTransfer] == VK_QUEUE_FAMILY_IGNORED)
            && HasAnyBit(queueFamilyProps[i].queueFlags, VK_QUEUE_TRANSFER_BIT)
            && HasNoBit(queueFamilyProps[i].queueFlags, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
        {
            m_queueFamilyIndices[VulkanQueueFamilyTransfer] = i;
            VkDeviceQueueCreateInfo queueInfo = {};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.pNext;
            queueInfo.flags;
            queueInfo.queueFamilyIndex = i;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = queuePriorities;
            queueInfos.push_back(queueInfo);
        }
    }

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledLayerCount = 0; // is deprecated and ignored.
    createInfo.ppEnabledLayerNames = nullptr; // is deprecated and ignored.
    m_enabledExtensionNames = extensionNames;
    std::vector<const char*> enabledExtensionNames;
    enabledExtensionNames.reserve(m_enabledExtensionNames.size());
    for (const std::string& extensionName : m_enabledExtensionNames)
    {
        enabledExtensionNames.push_back(extensionName.data());
    }
    createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensionNames.size());
    createInfo.ppEnabledExtensionNames = enabledExtensionNames.data();
    createInfo.pEnabledFeatures = nullptr;

    const VkPhysicalDeviceFeatures2& features2 = m_physicalDevice->GetFeatures2();
    createInfo.pNext = &features2;

    VK_CHECK(vkCreateDevice(m_physicalDevice->GetHandle(), &createInfo, nullptr, &m_handle));
    if (m_handle)
    {
        volkLoadDeviceTable(&m_functionTable, m_handle);

        // Vma Initialization
        // https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/quick_start.html#quick_start_initialization
        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.vulkanApiVersion = m_instance->GetApiVersion();
        allocatorCreateInfo.instance = m_instance->GetHandle();
        allocatorCreateInfo.physicalDevice = m_physicalDevice->GetHandle();
        allocatorCreateInfo.device = m_handle;
        VmaVulkanFunctions vmaFunctions = {};
        vmaFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vmaFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        allocatorCreateInfo.pVulkanFunctions = &vmaFunctions;
        if (m_physicalDevice->GetVulkan12Features().bufferDeviceAddress)
        {
            allocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        }
        VK_CHECK(vmaCreateAllocator(&allocatorCreateInfo, &m_allocator));
    }

    for (uint32_t i = 0; i < VulkanQueueFamilyCount; i++)
    {
        VulkanQueueFamily queueFamily = static_cast<VulkanQueueFamily>(i);
        m_queues[i] = new VulkanQueue(this, queueFamily);
        m_commandPoolsTransientAlloc[queueFamily] = CreateCommandPool(queueFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    }
}

VulkanDevice::~VulkanDevice()
{
    vmaDestroyAllocator(m_allocator);
    m_allocator = VK_NULL_HANDLE;
    GetFunctionTable()->
        vkDestroyDevice(m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}

const VkPhysicalDeviceLimits& VulkanDevice::GetLimits() const
{
    return m_physicalDevice->GetProperties().limits;
}

bool VulkanDevice::SupportsExtension(std::string_view extension)
{
    for (const auto& extensionName : m_enabledExtensionNames)
    {
        if (extensionName == extension)
        {
            return true;
        }
    }
    return false;
}

uint32_t VulkanDevice::GetQueueFamilyIndex(VulkanQueueFamily queueFamily)
{
    return m_queueFamilyIndices[queueFamily];
}

VulkanQueue* VulkanDevice::GetQueue(VulkanQueueFamily queueFamily)
{
    return m_queues[queueFamily].get();
}

bool VulkanDevice::SupportsSurface(VulkanQueueFamily queueFamily, VkSurfaceKHR surface) const
{
    return m_physicalDevice->SupportsSurface(m_queueFamilyIndices[queueFamily], surface);
}

Ref<VulkanCommandPool> VulkanDevice::CreateCommandPool(VulkanQueueFamily queueFamily, VkCommandPoolCreateFlags flags)
{
    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = flags;
    createInfo.queueFamilyIndex = GetQueueFamilyIndex(queueFamily);
    return new VulkanCommandPool(this, createInfo);
}

Ref<VulkanCommandBuffer> VulkanDevice::AllocateCommandBufferOneTimeUse(VulkanQueueFamily queueFamily)
{
    return m_commandPoolsTransientAlloc[queueFamily]->Allocate();
}

Ref<VulkanFence> VulkanDevice::CreateFence(VkFenceCreateFlags flags)
{
    VkFenceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = flags;
    return new VulkanFence(this, createInfo);
}

Ref<VulkanSemaphore> VulkanDevice::CreateSemaphore(VkSemaphoreCreateFlags flags)
{
    VkSemaphoreCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0; // reserved for future use
    return new VulkanSemaphore(this, createInfo);
}

Ref<VulkanSemaphore> VulkanDevice::CreateSemaphoreSignaled()
{
    return CreateSemaphore(VK_FENCE_CREATE_SIGNALED_BIT);
}

Ref<VulkanSemaphore> VulkanDevice::CreateSemaphoreFromHandle(VkSemaphore semaphoreHandle)
{
    return new VulkanSemaphore(this, semaphoreHandle);
}

Ref<VulkanEvent> VulkanDevice::CreateEvent()
{
    VkEventCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0; // reserved for future use
    return new VulkanEvent(this, createInfo);
}

void VulkanDevice::WaitIdle()
{
    VK_CHECK(GetFunctionTable()->
        vkDeviceWaitIdle(m_handle));
}

Ref<VulkanRenderPass> VulkanDevice::CreateRenderPass(const VkRenderPassCreateInfo& createInfo)
{
    return new VulkanRenderPass(this, createInfo);
}

Ref<VulkanFramebuffer> VulkanDevice::CreateFramebuffer(VulkanRenderPass* renderPass, ArrayRef<VulkanImageView*> attachments, uint32_t width, uint32_t height, uint32_t layers)
{
    std::vector<VkImageView> attachmenstHandles(attachments.size());
    for (int i = 0; i < attachments.size(); i++) {
        attachmenstHandles[i] = attachments[i]->GetHandle();
    }

    VkFramebufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.renderPass = renderPass->GetHandle();
    createInfo.attachmentCount = static_cast<uint32_t>(attachmenstHandles.size());
    createInfo.pAttachments = attachmenstHandles.data();
    createInfo.width = width;
    createInfo.height = height;
    createInfo.layers = layers;

    return new VulkanFramebuffer(this, createInfo);
}

Ref<VulkanShader> VulkanDevice::CreateShader(
    VkShaderStageFlagBits       stage,
    std::string_view            fileName,
    std::string_view            source,
    std::string_view            entryPoint,
    ArrayRef<ShaderMacro>       macros)
{
    Ref<VulkanShader> shader = new VulkanShader(stage);
    if (shader->Compile(fileName, source, entryPoint, macros))
    {
        return shader;
    }
    else
    {
        LogPrint("Vulkan", LogLevel::Error, "Shader compile failed: fileName: %s:\n %s",
            fileName.data(), shader->GetLog());
        return nullptr;
    }
}

Ref<VulkanShader> VulkanDevice::CreateShaderFromFile(
    VkShaderStageFlagBits       stage,
    std::string_view            fileName,
    std::string_view            entryPoint,
    ArrayRef<ShaderMacro>       macros)
{
    std::string source = File::ReadAll(fileName);
    return CreateShader(stage, fileName, source, entryPoint, macros);
}

Ref<VulkanShaderModule> VulkanDevice::CreateShaderModule(ArrayRef<uint32_t> code)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    return new VulkanShaderModule(this, createInfo);
}

Ref<VulkanGraphicsPipeline> VulkanDevice::CreateGraphicsPipeline(const VulkanGraphicsPipelineCreateInfo& createInfo)
{
    return new VulkanGraphicsPipeline(this, createInfo);
}

Ref<VulkanComputePipeline> VulkanDevice::CreateComputePipeline(const VulkanComputePipelineCreateInfo& createInfo)
{
    return new VulkanComputePipeline(this, createInfo);
}

Ref<VulkanBuffer> VulkanDevice::CreateBuffer(const VulkanBufferCreateInfo& createInfo)
{
    return new VulkanBuffer(this, createInfo);
}

Ref<VulkanBuffer> VulkanDevice::CreateUniformBuffer(VkDeviceSize size, bool isPersistentMapped)
{
    VulkanBufferCreateInfo createInfo;
    createInfo.SetUniformBuffer(size, isPersistentMapped);
    return new VulkanBuffer(this, createInfo);
}

Ref<VulkanBuffer> VulkanDevice::CreateStorageBuffer(VkDeviceSize size, VkBufferUsageFlags additionalUsage)
{
    VulkanBufferCreateInfo createInfo;
    createInfo.SetStorageBuffer(size);
    createInfo.m_createInfo.usage |= additionalUsage;
    return new VulkanBuffer(this, createInfo);
}

Ref<VulkanBuffer> VulkanDevice::CreateIndexBuffer(VkDeviceSize size, VkBufferUsageFlags additionalUsage)
{
    VulkanBufferCreateInfo createInfo;
    createInfo.SetIndexBuffer(size);
    createInfo.m_createInfo.usage |= additionalUsage;
    return new VulkanBuffer(this, createInfo);
}

Ref<VulkanBuffer> VulkanDevice::CreateVertexBuffer(VkDeviceSize size, VkBufferUsageFlags additionalUsage)
{
    VulkanBufferCreateInfo createInfo;
    createInfo.SetVertexBuffer(size);
    createInfo.m_createInfo.usage |= additionalUsage;
    return new VulkanBuffer(this, createInfo);
}

Ref<VulkanBuffer> VulkanDevice::CreateStagingBuffer(VkDeviceSize size)
{
    VulkanBufferCreateInfo createInfo;
    createInfo.SetStagingBuffer(size);
    return new VulkanBuffer(this, createInfo);
}

Ref<VulkanImage> VulkanDevice::CreateImage(const VulkanImageCreateInfo& createInfo)
{
    return new VulkanImage(this, createInfo);
}

Ref<VulkanImage> VulkanDevice::CreateImageFromHandle(VkImage imageHandle, const VkImageCreateInfo& createInfo)
{
    return new VulkanImage(this, imageHandle, createInfo);
}

Ref<VulkanImage> VulkanDevice::CreateImage2D_DepthStencilAttachment(VkFormat format, uint32_t width, uint32_t height)
{
    VulkanImageCreateInfo createInfo = {};
    createInfo.SetDepthStencil(format, width, height);
    return CreateImage(createInfo);
}

Ref<VulkanSampler> VulkanDevice::CreatSampler(const VkSamplerCreateInfo& createInfo)
{
    return new VulkanSampler(this, createInfo);
}

Ref<VulkanDescriptorSetLayout> VulkanDevice::CreateDescriptorSetLayout(ArrayRef<VkDescriptorSetLayoutBinding> layoutBindings)
{
    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    createInfo.pBindings = layoutBindings.data();

    return new VulkanDescriptorSetLayout(this, createInfo);
}

Ref<VulkanPipelineLayout> VulkanDevice::CreatePipelineLayout(
    ArrayRef<VulkanDescriptorSetLayout*> descSetLayouts, ArrayRef<VkPushConstantRange> pushConstantRanges)
{
    std::vector<VkDescriptorSetLayout> descSetLayoutsHandles(descSetLayouts.size());
    for (int i = 0; i < descSetLayouts.size(); i++)
    {
        descSetLayoutsHandles[i] = descSetLayouts[i]->GetHandle();
    }

    VkPipelineLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.setLayoutCount = static_cast<uint32_t>(descSetLayoutsHandles.size());
    createInfo.pSetLayouts = descSetLayoutsHandles.data();
    createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    createInfo.pPushConstantRanges = pushConstantRanges.data();

    return new VulkanPipelineLayout(this, createInfo);
}

Ref<VulkanDescriptorPool> VulkanDevice::CreateDescriptorPool(uint32_t maxSets, ArrayRef<VkDescriptorPoolSize> poolSizes)
{
    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    createInfo.maxSets = maxSets;
    createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    createInfo.pPoolSizes = poolSizes.data();

    return new VulkanDescriptorPool(this, createInfo);
}

Ref<VulkanSwapchain> VulkanDevice::CreateSwapchain(const VkSwapchainCreateInfoKHR& createInfo)
{
    return new VulkanSwapchain(this, createInfo);
}
