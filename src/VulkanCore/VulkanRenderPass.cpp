#include "rad/VulkanCore/VulkanRenderPass.h"
#include "rad/VulkanCore/VulkanDevice.h"

VulkanRenderPass::VulkanRenderPass(Ref<VulkanDevice> device, const VkRenderPassCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateRenderPass(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

VulkanRenderPass::~VulkanRenderPass()
{
    m_device->GetFunctionTable()->
        vkDestroyRenderPass(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}
