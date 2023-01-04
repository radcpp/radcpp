#include "rad/VulkanCore/VulkanSampler.h"
#include "rad/VulkanCore/VulkanDevice.h"

VulkanSampler::VulkanSampler(Ref<VulkanDevice> device, const VkSamplerCreateInfo & createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateSampler(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

VulkanSampler::~VulkanSampler()
{
    m_device->GetFunctionTable()->
        vkDestroySampler(m_device->GetHandle(), m_handle, nullptr);
}
