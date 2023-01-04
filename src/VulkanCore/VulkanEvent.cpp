#include "rad/VulkanCore/VulkanEvent.h"
#include "rad/VulkanCore/VulkanDevice.h"

VulkanEvent::VulkanEvent(Ref<VulkanDevice> device, const VkEventCreateInfo & createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateEvent(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

VulkanEvent::~VulkanEvent()
{
    m_device->GetFunctionTable()->
        vkDestroyEvent(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}
