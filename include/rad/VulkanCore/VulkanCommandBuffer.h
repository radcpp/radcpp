#ifndef VULKAN_COMMAND_BUFFER_H
#define VULKAN_COMMAND_BUFFER_H
#pragma once

#include "VulkanCommon.h"

class VulkanCommandBuffer : public VulkanObject
{
public:
    VulkanCommandBuffer(Ref<VulkanDevice> device, Ref<VulkanCommandPool> commandPool, VkCommandBufferLevel level);
    ~VulkanCommandBuffer();

    VkCommandBuffer GetHandle() const { return m_handle; }

    void Reset(VkCommandBufferResetFlags flags = 0);

    // Recording

    void Begin(VkCommandBufferUsageFlags flags = 0, const VkCommandBufferInheritanceInfo* pInheritanceInfo = nullptr);
    void End();

    void BeginRenderPass(const VkRenderPassBeginInfo& beginInfo, VkSubpassContents contents);
    void BeginRenderPass(VulkanRenderPass* renderPass, VulkanFramebuffer* framebuffer,
        const VkRect2D& renderArea, ArrayRef<VkClearValue> clearValues, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
    void BeginRenderPass(VulkanRenderPass* renderPass, VulkanFramebuffer* framebuffer, ArrayRef<VkClearValue> clearValues);
    void EndRenderPass();
    void NextSubpass(VkSubpassContents contents);

    void BindPipeline(VulkanPipeline* pipeline);
    void BindDescriptorSets(VulkanPipeline* pipeline, VulkanPipelineLayout* layout,
        uint32_t firstSet, ArrayRef<VulkanDescriptorSet*> descSets,
        ArrayRef<uint32_t> dynamicOffsets = {});

    void SetScissors(ArrayRef<VkRect2D> scissors, uint32_t first = 0);
    void SetViewports(ArrayRef<VkViewport> viewports, uint32_t first = 0);

    void SetDepthBounds(float min, float max);

    void SetDepthStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask);
    void SetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask);
    void SetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference);

    void SetBlendConstants(float r, float g, float b, float a);

    // Rasterization

    void SetLineWidth(float width);
    void SetDepthBias(float constantFactor, float clamp, float slopeFactor);

    void BindIndexBuffer(VulkanBuffer* buffer, VkDeviceSize offset, VkIndexType indexType);
    void BindVertexBuffers(uint32_t firstBinding, ArrayRef<VulkanBuffer*> buffers, ArrayRef<VkDeviceSize> offsets);
    void BindVertexBuffers(uint32_t firstBinding, ArrayRef<VulkanBuffer*> buffers);

    // Draw

    void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    void DrawIndexed(
        uint32_t indexCount, uint32_t instanceCount,
        uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void DrawIndirect(VulkanBuffer* buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
    void DrawIndexedIndirect(VulkanBuffer* buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);

    // Dispatch

    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
    void DispatchIndirect(VulkanBuffer* buffer, VkDeviceSize offset);
    void DispatchBase(
        uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
        uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    // Clear

    void ClearColorImage(
        VulkanImage* image, VkImageLayout layout,
        const VkClearColorValue& color, ArrayRef<VkImageSubresourceRange> ranges);
    void ClearDepthStencilImage(
        VulkanImage* image, VkImageLayout layout,
        const VkClearDepthStencilValue& value, ArrayRef<VkImageSubresourceRange> ranges);
    void ClearAttachments(ArrayRef<VkClearAttachment> attachments, ArrayRef<VkClearRect> rects);
    void FillBuffer(VulkanBuffer* dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);
    // dataSize must be less than or equal to 65536 bytes. For larger updates, applications can use buffer to buffer copies.
    // Buffer updates performed with vkCmdUpdateBuffer first copy the data into command buffer memory
    // when the command is recorded (which requires additional storage and may incur an additional allocation),
    // and then copy the data from the command buffer into dstBuffer when the command is executed on a device.
    void UpdateBuffer(VulkanBuffer* dstBuffer, VkDeviceSize offset, VkDeviceSize dataSize, const uint32_t* pData);

    // Copy

    void CopyBuffer(VulkanBuffer* srcBuffer, VulkanBuffer* dstBuffer, ArrayRef<VkBufferCopy> regions);
    void CopyImage(VulkanImage* srcImage, VkImageLayout srcLayout,
        VulkanImage* dstImage, VkImageLayout dstLayout,
        ArrayRef<VkImageCopy> regions);
    void CopyBufferToImage(
        VulkanBuffer* srcBuffer,
        VulkanImage* dstImage, VkImageLayout dstImageLayout,
        ArrayRef<VkBufferImageCopy> regions);
    void CopyImageToBuffer(VulkanImage* srcImage, VkImageLayout srcImageLayout,
        VulkanBuffer* dstBuffer, ArrayRef<VkBufferImageCopy> regions);
    void BlitImage(
        VulkanImage* srcImage, VkImageLayout srcImageLayout,
        VulkanImage* dstImage, VkImageLayout dstImageLayout,
        ArrayRef<VkImageBlit> regions, VkFilter filter);
    void ResolveImage(
        VulkanImage* srcImage, VkImageLayout srcImageLayout,
        VulkanImage* dstImage, VkImageLayout dstImageLayout,
        ArrayRef<VkImageResolve> regions);

    void SetPushConstants(VulkanPipelineLayout* layout, VkShaderStageFlags stageFlags,
        uint32_t offset, uint32_t size, const void* pValues);

    // Khronos Synchronization Examples: https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples

    void SetPipelineBarrier(
        VkPipelineStageFlags                srcStageMask,
        VkPipelineStageFlags                dstStageMask,
        VkDependencyFlags                   dependencyFlags,
        ArrayRef<VkMemoryBarrier>           memoryBarriers,
        ArrayRef<VkBufferMemoryBarrier>     bufferMemoryBarriers,
        ArrayRef<VkImageMemoryBarrier>      imageMemoryBarriers
    );
    // Global memory barrier covers all resources.
    // Generally considered more efficient to do a global memory barrier than per-resource barriers, 
    // per-resource barriers should usually be used for queue ownership transfers and image layout transitions -
    // otherwise use global barriers.
    void SetMemoryBarrier(
        VkPipelineStageFlags    srcStageMask,
        VkPipelineStageFlags    dstStageMask,
        VkAccessFlags           srcAccessMask,
        VkAccessFlags           dstAccessMask);
    void SetMemoryBarrier_ComputeToComputeReadAfterWrite();
    // Write-After-Read hazards don't need a memory barrier between them - execution barriers are sufficient.
    // A pipeline barrier or event without a memory barrier is an execution-only dependency.
    void SetMemoryBarrier_ComputeToComputeExecutionBarrier();
    void SetMemoryBarrier_ComputeToIndexInput();
    void SetMemoryBarrier_ComputeToDrawIndirect();
    // Dispatch writes into a storage image. Draw samples that image in a fragment shader.
    void SetStorageImageBarrier_ComputeToFragmentShaderSample(VulkanImage* image);

    // Only use this for debugging -
    // this is not something that should ever ship in real code, this will flush and invalidate all caches and stall everything,
    // it is a tool not to be used lightly!
    // That said, it can be really handy if you think you have a race condition in your app
    // and you just want to serialize everything so you can debug it.
    // Note that this does not take care of image layouts -
    // if you're debugging you can set the layout of all your images to GENERAL to overcome this,
    // but again - do not do this in release code!
    void SetFullPipelineBarrier();

    void TransitLayout(
        VulkanImage*            image,
        VkPipelineStageFlags    srcStageMask,
        VkPipelineStageFlags    dstStageMask,
        VkAccessFlags           srcAccessMask,
        VkAccessFlags           dstAccessMask,
        VkImageLayout           oldLayout,
        VkImageLayout           newLayout,
        const VkImageSubresourceRange* subresourceRange = nullptr);

private:
    Ref<VulkanDevice>           m_device;
    Ref<VulkanCommandPool>      m_commandPool;
    VkCommandBuffer             m_handle = VK_NULL_HANDLE;
    VkCommandBufferLevel        m_level;

}; // class VulkanCommandBuffer


#endif // VULKAN_COMMAND_BUFFER_H