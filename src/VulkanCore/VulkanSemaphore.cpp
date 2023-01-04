#include "rad/VulkanCore/VulkanSemaphore.h"
#include "rad/VulkanCore/VulkanDevice.h"

VulkanSemaphore::VulkanSemaphore(Ref<VulkanDevice> device, const VkSemaphoreCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateSemaphore(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
    m_isManaged = true;
}

VulkanSemaphore::VulkanSemaphore(Ref<VulkanDevice> device, VkSemaphore handle) :
    m_device(std::move(device)),
    m_handle(handle)
{
    m_isManaged = false;
}

VulkanSemaphore::~VulkanSemaphore()
{
    if (m_isManaged)
    {
        m_device->GetFunctionTable()->
            vkDestroySemaphore(m_device->GetHandle(), m_handle, nullptr);
    }
    m_handle = VK_NULL_HANDLE;
}
