#include "rad/VulkanCore/VulkanDescriptorPool.h"
#include "rad/VulkanCore/VulkanDevice.h"
#include "rad/VulkanCore/VulkanDescriptorSet.h"

VulkanDescriptorPool::VulkanDescriptorPool(Ref<VulkanDevice> device, const VkDescriptorPoolCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateDescriptorPool(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
    m_device->GetFunctionTable()->
        vkDestroyDescriptorPool(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}

Ref<VulkanDescriptorSet> VulkanDescriptorPool::Allocate(VulkanDescriptorSetLayout* layout)
{
    return new VulkanDescriptorSet(m_device, this, layout);
}

void VulkanDescriptorPool::Reset()
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkResetDescriptorPool(m_device->GetHandle(), m_handle, 0));
}
