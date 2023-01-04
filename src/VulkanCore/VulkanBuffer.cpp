#include "rad/VulkanCore/VulkanBuffer.h"
#include "rad/VulkanCore/VulkanDevice.h"
#include "rad/VulkanCore/VulkanQueue.h"
#include "rad/VulkanCore/VulkanCommandBuffer.h"

VulkanBuffer::VulkanBuffer(Ref<VulkanDevice> device, const VulkanBufferCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(vmaCreateBuffer(m_device->GetAllocator(), &createInfo.m_createInfo, &createInfo.m_allocation,
        &m_handle, &m_allocation, &m_allocationInfo));

    m_size = createInfo.m_createInfo.size;
    m_usage = createInfo.m_createInfo.usage;
    m_sharingMode = createInfo.m_createInfo.sharingMode;

    vmaGetMemoryTypeProperties(m_device->GetAllocator(), m_allocationInfo.memoryType, &m_memoryFlags);
}

VulkanBuffer::~VulkanBuffer()
{
    vmaDestroyBuffer(m_device->GetAllocator(), m_handle, m_allocation);
}

VkDeviceAddress VulkanBuffer::GetDeviceAddress() const
{
    VkBufferDeviceAddressInfo deviceAddressInfo = {};
    deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    deviceAddressInfo.pNext = nullptr;
    deviceAddressInfo.buffer = m_handle;
    return vkGetBufferDeviceAddress(m_device->GetHandle(), &deviceAddressInfo);
}

void* VulkanBuffer::GetPersistentMappedAddr()
{
    return m_allocationInfo.pMappedData;
}

void* VulkanBuffer::MapMemory(VkDeviceSize offset, VkDeviceSize size)
{
    if (HasAnyBit(m_memoryFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
    {
        void* pMappedAddr = nullptr;
        VK_CHECK(vmaMapMemory(m_device->GetAllocator(), m_allocation, &pMappedAddr));
        if (!HasAnyBit(m_memoryFlags, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        {
            vmaInvalidateAllocation(m_device->GetAllocator(), m_allocation, offset, size);
        }
        return pMappedAddr;
    }
    else
    {
        return nullptr;
    }
}

void VulkanBuffer::UnmapMemory()
{
    vmaUnmapMemory(m_device->GetAllocator(), m_allocation);
}

void VulkanBuffer::Read(void* dest, VkDeviceSize offset, VkDeviceSize size)
{
    if (HasAnyBit(m_memoryFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
    {
        void* pMappedAddr = nullptr;
        VK_CHECK(vmaMapMemory(m_device->GetAllocator(), m_allocation, &pMappedAddr));
        if (!HasAnyBit(m_memoryFlags, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        {
            vmaInvalidateAllocation(m_device->GetAllocator(), m_allocation, offset, size);
        }
        memcpy(dest, pMappedAddr, size);
        vmaUnmapMemory(m_device->GetAllocator(), m_allocation);
    }
    else
    {
        Ref<VulkanCommandBuffer> commandBuffer =
            m_device->AllocateCommandBufferOneTimeUse();

        Ref<VulkanBuffer> stagingBuffer = m_device->CreateStagingBuffer(size);
        commandBuffer->Begin();
        VkMemoryBarrier transferReadBarrier = {};
        transferReadBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        transferReadBarrier.pNext;
        transferReadBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        transferReadBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = offset;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        commandBuffer->SetPipelineBarrier(
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            { &transferReadBarrier, 1 }, {}, {});
        commandBuffer->CopyBuffer(this, stagingBuffer.get(), { &copyRegion, 1 });
        VkBufferMemoryBarrier hostReadBarrier = {};
        hostReadBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        hostReadBarrier.pNext;
        hostReadBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        hostReadBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
        hostReadBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        hostReadBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        hostReadBarrier.buffer = stagingBuffer->GetHandle();
        hostReadBarrier.offset = 0;
        hostReadBarrier.size = size;
        commandBuffer->SetPipelineBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_HOST_BIT,
            0,
            {}, { &hostReadBarrier, 1 }, {});
        commandBuffer->End();

        m_device->GetQueue()->SubmitAndWaitForCompletion({ commandBuffer.get() });

        stagingBuffer->Read(dest);
    }
}

void VulkanBuffer::Read(void* dest)
{
    Read(dest, 0, GetSize());
}

void VulkanBuffer::Write(const void* data, VkDeviceSize offset, VkDeviceSize size)
{
    assert(offset + size <= m_size);
    if (HasAnyBit(m_memoryFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
    {
        void* pMappedAddr = nullptr;
        VK_CHECK(vmaMapMemory(m_device->GetAllocator(), m_allocation, &pMappedAddr));
        if (!HasAnyBit(m_memoryFlags, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        {
            vmaFlushAllocation(m_device->GetAllocator(), m_allocation, offset, size);
        }
        memcpy(pMappedAddr, data, size);
        vmaUnmapMemory(m_device->GetAllocator(), m_allocation);
    }
    else
    {
        Ref<VulkanCommandBuffer> commandBuffer = m_device->AllocateCommandBufferOneTimeUse();

        Ref<VulkanBuffer> stagingBuffer = m_device->CreateStagingBuffer(size);
        stagingBuffer->Write(data);
        commandBuffer->Begin();
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = offset;
        copyRegion.size = size;
        commandBuffer->CopyBuffer(stagingBuffer.get(), this, { &copyRegion, 1 });
        commandBuffer->End();

        m_device->GetQueue()->SubmitAndWaitForCompletion({ commandBuffer.get() });
    }
}

void VulkanBuffer::Write(const void* data)
{
    Write(data, 0, GetSize());
}

Ref<VulkanBufferView> VulkanBuffer::CreateBufferView(VkFormat format, VkDeviceSize offset, VkDeviceSize range)
{
    VkBufferViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.buffer = m_handle;
    createInfo.format = format;
    createInfo.offset = offset;
    createInfo.range = range;

    return new VulkanBufferView(m_device, this, createInfo);
}

VulkanBufferView::VulkanBufferView(Ref<VulkanDevice> device, Ref<VulkanBuffer> buffer, const VkBufferViewCreateInfo& createInfo) :
    m_device(std::move(device)),
    m_buffer(std::move(buffer))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateBufferView(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
    m_format = createInfo.format;
    m_offset = createInfo.offset;
    m_range = createInfo.range;
}

VulkanBufferView::~VulkanBufferView()
{
    m_device->GetFunctionTable()->
        vkDestroyBufferView(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}
