#include "rad/VulkanCore/VulkanWindow.h"
#include "rad/VulkanCore.h"

#include "SDL2/SDL_vulkan.h"

VulkanWindow::VulkanWindow(Ref<VulkanInstance> instance) :
    m_instance(std::move(instance))
{
}

VulkanWindow::~VulkanWindow()
{
}

bool VulkanWindow::Create(const char* title, int x, int y, int width, int height, Uint32 flags)
{
    if (!Window::Create(title, x, y, width, height, flags | SDL_WINDOW_VULKAN))
    {
        return false;
    }

    if (!SDL_Vulkan_CreateSurface(m_handle, m_instance->GetHandle(), &m_surface))
    {
        LogPrint("Vulkan", LogLevel::Error, "VulkanWindow: SDL_Vulkan_CreateSurface failed: %s", SDL_GetError());
        return false;
    }

    return true;
}

void VulkanWindow::Destroy()
{
    if (m_device)
    {
        m_device->WaitIdle();
    }

    if (m_swapchain)
    {
        m_swapchain.reset();
    }

    vkDestroySurfaceKHR(m_instance->GetHandle(), m_surface, nullptr);

    Window::Destroy();
}

bool VulkanWindow::Init(Ref<VulkanDevice> device)
{
    m_device = device;

    int width = 0;
    int height = 0;
    GetDrawableSize(&width, &height);
    CreateSizeDependentResources(width, height);

    m_commandPool = m_device->CreateCommandPool(VulkanQueueFamilyUniversal,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    m_commandBuffers.resize(m_swapchainImageCount);
    for (uint32_t i = 0; i < m_swapchainImageCount; i++)
    {
        m_commandBuffers[i] = m_commandPool->Allocate();
    }

    for (uint32_t i = 0; i < FrameLag; i++)
    {
        m_swapchainImageAcquired[i] = m_device->CreateSemaphore();
        m_drawComplete[i] = m_device->CreateSemaphore();
        m_frameThrottles[i] = m_device->CreateFence(VK_FENCE_CREATE_SIGNALED_BIT);
    }

    return true;
}

void VulkanWindow::GetDrawableSize(int* w, int* h)
{
    SDL_Vulkan_GetDrawableSize(m_handle, w, h);
}

VulkanCommandBuffer* VulkanWindow::BeginFrame()
{
    m_frameThrottles[m_frameIndex]->Wait();
    m_frameThrottles[m_frameIndex]->Reset();
    m_swapchainImageIndex = m_swapchain->AcquireNextImage(m_swapchainImageAcquired[m_frameIndex].get());

    return m_commandBuffers[m_swapchainImageIndex].get();
}

void VulkanWindow::EndFrame()
{
    m_device->GetQueue()->Submit(
        std::array{
            m_commandBuffers[m_swapchainImageIndex].get()
        },
        std::array{ // wait
            VulkanSubmitWait{m_swapchainImageAcquired[m_frameIndex].get(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}
        },
        std::array{ // signal
            m_drawComplete[m_frameIndex].get()
        },
        m_frameThrottles[m_frameIndex].get()
    );
    m_device->GetQueue()->Present(
        std::array{ // wait
            m_drawComplete[m_frameIndex].get()
        },
        std::array{
            m_swapchain.get()
        },
        std::array{
            m_swapchainImageIndex
        }
    );

    m_frameIndex += 1;
    m_frameIndex %= FrameLag;
}

VulkanCommandBuffer* VulkanWindow::GetCommandBuffer()
{
    return m_commandBuffers[m_swapchainImageIndex].get();
}

void VulkanWindow::OnResized(int width, int height)
{
    m_device->WaitIdle();

    int drawableWidth = 0;
    int drawableHeight = 0;
    GetDrawableSize(&drawableWidth, &drawableHeight);
    CreateSizeDependentResources(drawableWidth, drawableHeight);
}

void VulkanWindow::OnClose()
{
    m_shouldClose = true;
    Destroy();
}

bool VulkanWindow::CreateSizeDependentResources(uint32_t width, uint32_t height)
{
    CreateSwapchain(width, height);
    CreateDepthStencil(width, height);
    CreateDefaultRenderPass();
    CreateDefaultFramebuffers(width, height);
    return true;
}

VkSurfaceFormatKHR PickSurfaceFormat(ArrayRef<VkSurfaceFormatKHR> surfaceFormats)
{
    assert(surfaceFormats.size() >= 1);

    VkSurfaceFormatKHR defaultFormat = {};
    defaultFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    defaultFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    if (surfaceFormats.empty())
    {
        return defaultFormat;
    }

    if ((surfaceFormats.size() == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
    {
        return defaultFormat;
    }

    for (size_t i = 0; i < surfaceFormats.size(); i++)
    {
        const VkFormat format = surfaceFormats[i].format;

        if (format == VK_FORMAT_R8G8B8A8_UNORM || format == VK_FORMAT_B8G8R8A8_UNORM ||
            format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 || format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 ||
            format == VK_FORMAT_R16G16B16A16_SFLOAT)
        {
            return surfaceFormats[i];
        }
    }

    LogPrint("Vulkan", LogLevel::Warn, "Can't find our preferred formats... Falling back to first exposed format. Rendering may be incorrect.\n");
    return surfaceFormats[0];
}

bool VulkanWindow::CreateSwapchain(uint32_t width, uint32_t height)
{
    if (m_swapchain &&
        (m_swapchain->GetWidth() == width) &&
        (m_swapchain->GetHeight() == height))
    {
        return true;
    }

    m_surfaceCapabilities = m_device->GetPhysicalDevice()->GetSurfaceCapabilities(m_surface);
    std::vector<VkSurfaceFormatKHR> surfaceFormats = m_device->GetPhysicalDevice()->GetSurfaceFormats(m_surface);
    std::vector<VkPresentModeKHR> presentModes = m_device->GetPhysicalDevice()->GetSurfacePresentModes(m_surface);

    // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
    if (m_surfaceCapabilities.currentExtent.width == 0xFFFFFFFF)
    {
        // If the surface size is undefined, the size is set to the size of the images requested -
        // which must fit within the minimum and maximum values.
        if (width < m_surfaceCapabilities.minImageExtent.width)
        {
            width = m_surfaceCapabilities.minImageExtent.width;
        }
        else if (width > m_surfaceCapabilities.maxImageExtent.width)
        {
            width = m_surfaceCapabilities.maxImageExtent.width;
        }

        if (height < m_surfaceCapabilities.minImageExtent.height)
        {
            height = m_surfaceCapabilities.minImageExtent.height;
        }
        else if (height > m_surfaceCapabilities.minImageExtent.height)
        {
            height = m_surfaceCapabilities.minImageExtent.height;
        }
    }
    else
    {
        // If the surface size is defined, the swap chain size must match
        width = m_surfaceCapabilities.minImageExtent.width;
        height = m_surfaceCapabilities.minImageExtent.height;
    }

    if (width == 0 || height == 0)
    {
        return false;
    }

    if (m_swapchainImageCount < m_surfaceCapabilities.minImageCount)
    {
        m_swapchainImageCount = m_surfaceCapabilities.minImageCount;
    }
    if ((m_surfaceCapabilities.maxImageCount > 0) && (m_swapchainImageCount > m_surfaceCapabilities.maxImageCount))
    {
        m_swapchainImageCount = m_surfaceCapabilities.maxImageCount;
    }

    m_surfaceFormat = PickSurfaceFormat(surfaceFormats);

    VkColorSpaceKHR colorSpace = surfaceFormats[0].colorSpace;

    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    if (m_surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        preTransform = m_surfaceCapabilities.currentTransform;
    }

    VkCompositeAlphaFlagBitsKHR compositeAlpha = HasAnyBit(m_surfaceCapabilities.supportedCompositeAlpha, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (m_surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
    {
        compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
    }
    else if (m_surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
    {
        compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!presentModes.empty())
    {
        presentMode = presentModes[0];
    }
    if (m_enableVSync)
    {
        presentMode = VK_PRESENT_MODE_FIFO_KHR; // guaranteed to be supported
    }

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = nullptr;
    swapchainInfo.flags = 0;
    swapchainInfo.surface = m_surface;
    swapchainInfo.minImageCount = m_swapchainImageCount;
    swapchainInfo.imageFormat = m_surfaceFormat.format;
    swapchainInfo.imageColorSpace = colorSpace;
    swapchainInfo.imageExtent.width = width;
    swapchainInfo.imageExtent.height = height;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.queueFamilyIndexCount;
    swapchainInfo.pQueueFamilyIndices;
    swapchainInfo.preTransform = preTransform;
    swapchainInfo.compositeAlpha = compositeAlpha;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = m_swapchain ? m_swapchain->GetHandle() : VK_NULL_HANDLE;

    m_swapchain = m_device->CreateSwapchain(swapchainInfo);
    LogPrint("Vulkan", LogLevel::Info, "VulkanSwapchain (re)created: handle=0x%p; extent=%ux%u;",
        m_swapchain->GetHandle(), m_swapchain->GetWidth(), m_swapchain->GetHeight());
    return true;
}

bool VulkanWindow::CreateDepthStencil(uint32_t width, uint32_t height)
{
    VulkanImageCreateInfo createInfo = {};
    createInfo.SetDepthStencil(m_depthStencilFormat, width, height);
    m_depthStencil = m_device->CreateImage(createInfo);
    return true;
}

bool VulkanWindow::CreateDefaultRenderPass()
{
    std::array<VkAttachmentDescription, 2> attachments = {};
    attachments[0].flags = 0;
    attachments[0].format = m_surfaceFormat.format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[1].flags = 0;
    attachments[1].format = m_depthStencilFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentReference depthStencilRef = {};
    depthStencilRef.attachment = 1;
    depthStencilRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDesc = {};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.flags = 0;
    subpassDesc.inputAttachmentCount = 0;
    subpassDesc.pInputAttachments = NULL;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorRef;
    subpassDesc.pResolveAttachments = NULL;
    subpassDesc.pDepthStencilAttachment = &depthStencilRef;
    subpassDesc.preserveAttachmentCount = 0;
    subpassDesc.pPreserveAttachments = NULL;

    std::array<VkSubpassDependency, 2> attachmentDependencies = {};
    // Depth buffer is shared between swapchain images
    attachmentDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    attachmentDependencies[0].dstSubpass = 0;
    attachmentDependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    attachmentDependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    attachmentDependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    attachmentDependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    attachmentDependencies[0].dependencyFlags = 0;
    // Image Layout Transition
    attachmentDependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    attachmentDependencies[1].dstSubpass = 0;
    attachmentDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    attachmentDependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    attachmentDependencies[1].srcAccessMask = 0;
    attachmentDependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    attachmentDependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext;
    renderPassInfo.flags;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(attachmentDependencies.size());
    renderPassInfo.pDependencies = attachmentDependencies.data();

    m_defaultRenderPass = m_device->CreateRenderPass(renderPassInfo);
    return true;
}

bool VulkanWindow::CreateDefaultFramebuffers(uint32_t width, uint32_t height)
{
    m_defaultFramebuffers.resize(m_swapchainImageCount);
    for (int i = 0; i < m_defaultFramebuffers.size(); i++)
    {
        m_defaultFramebuffers[i] = m_device->CreateFramebuffer(m_defaultRenderPass.get(),
            std::array{
                m_swapchain->GetDefaultView(i),
                m_depthStencil->GetDefaultView(),
            },
            width, height, 1);
    }
    return true;
}
