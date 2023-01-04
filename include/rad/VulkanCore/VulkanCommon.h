#ifndef VULKAN_COMMON_H
#define VULKAN_COMMON_H

// Loading Vulkan entrypoints directly from the driver can increase performance by skipping loader dispatch overhead.
// Each VulkanDevice has its own VolkDeviceTable.
#define VK_NO_PROTOTYPES 1

// Enable Vulkan beta extensions
#define VK_ENABLE_BETA_EXTENSIONS 1

// If you want VMA to fetch pointers to Vulkan functions dynamically using vkGetInstanceProcAddr, vkGetDeviceProcAddr (this is the option presented in the example below):
// Define VMA_STATIC_VULKAN_FUNCTIONS to 0, VMA_DYNAMIC_VULKAN_FUNCTIONS to 1.
// Provide pointers to these two functions via VmaVulkanFunctions::vkGetInstanceProcAddr, VmaVulkanFunctions::vkGetDeviceProcAddr.
// The library will fetch pointers to all other functions it needs internally.
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include "Volk/volk.h"
#include "vulkan/vk_enum_string_helper.h"

// VulkanMemoryAllocator (VMA) library manages memory for VulkanBuffer and VulkanImage.
#include "vma/vk_mem_alloc.h"

#include "rad/Core/Containers.h"
#include "rad/Core/Exception.h"
#include "rad/Core/Logging.h"
#include "rad/Core/Memory.h"
#include "rad/Core/String.h"

// Base of all Vulkan classes
class VulkanObject : public RefCounted<VulkanObject>
{
public:
    VulkanObject() {}
    virtual ~VulkanObject() {}
protected:
    VulkanObject(VulkanObject&&) = delete;
    VulkanObject& operator=(VulkanObject&&) = delete;

}; // class VulkanObject

class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanDevice;
class VulkanQueue;
class VulkanCommandBuffer;
class VulkanCommandPool;
class VulkanFence;
class VulkanSemaphore;
class VulkanEvent;
class VulkanRenderPass;
class VulkanFramebuffer;
class VulkanShader;
class VulkanShaderModule;
class VulkanPipeline;
class VulkanComputePipeline;
class VulkanGraphicsPipeline;
class VulkanBuffer;
class VulkanBufferView;
class VulkanImage;
class VulkanImageView;
class VulkanSampler;
class VulkanDescriptorPool;
class VulkanDescriptorSet;
class VulkanDescriptorSetLayout;
class VulkanPipelineLayout;
class VulkanSwapchain;

class VulkanError : public std::exception
{
public:
    VulkanError(VkResult result) : m_result(result) {}
    char const* what() const override { return string_VkResult(m_result); }
private:
    VkResult m_result;
}; // class VulkanError

// Check Vulkan return code and throw VulkanError if result<0
void ReportVulkanError(VkResult result, const char* function, const char* file, uint32_t line);
#define VK_CHECK(VulkanCall) { const VkResult _result = VulkanCall; if (_result < 0) { ReportVulkanError(_result, #VulkanCall, __FILE__, __LINE__); } }

std::string GetVulkanVersionString(uint32_t versionNumber);

class VulkanFormat
{
public:
    VulkanFormat(VkFormat format) :
        m_format(format)
    {
    }
    ~VulkanFormat()
    {
    }

    operator VkFormat() const { return m_format; }

    // Numeric
    // Formats with more then one numeric type (VK_FORMAT_D16_UNORM_S8_UINT) will return false
    bool IsUNORM() const;
    bool IsSNORM() const;
    bool IsUSCALED() const;
    bool IsSSCALED() const;
    bool IsUINT() const;
    bool IsSINT() const;
    bool IsSRGB() const;
    bool IsSFLOAT() const;
    bool IsUFLOAT() const;

    // Types from "Interpretation of Numeric Format" table (OpTypeFloat vs OpTypeInt)
    bool IsSampledInt() const;
    bool IsSampledFloat() const;

    bool IsCompressed_ASTC_HDR() const;
    bool IsCompressed_ASTC_LDR() const;
    bool IsCompressed_BC() const;
    bool IsCompressed_EAC() const;
    bool IsCompressed_ETC2() const;
    bool IsCompressed_PVRTC() const;
    bool IsCompressed() const;

    VkImageAspectFlags GetAspectFlags() const;
    bool IsDepthStencil() const;
    bool IsDepthOnly() const;
    bool IsStencilOnly() const;
    bool HasDepth() const;
    bool HasStencil() const;
    uint32_t GetDepthSizeInBits() const;
    uint32_t GetStencilSizeInBits() const;

    bool IsPacked() const;

    bool RequiresYcbcrConversion() const;
    bool IsXChromaSubsampled() const;
    bool IsYChromaSubsampled() const;

    bool IsSinglePlane_422() const;
    uint32_t GetPlaneCount() const;
    bool IsIsMultiplane() const;
    VkFormat GetMultiplaneCompatibleFormat(VkImageAspectFlags plane_aspect) const;
    VkExtent2D GetMultiplaneExtentDivisors(VkImageAspectFlags plane_aspect) const;

    uint32_t GetComponentCount() const;
    VkExtent3D GetTexelBlockExtent() const;
    uint32_t GetElementSizeInBytes(VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT) const;
    double GetTexelSizeInBytes(VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT) const;

    bool IsUndefined() const;
    bool IsBlockedImage() const;
    bool IsColor() const;

private:
    VkFormat m_format;

}; // class VulkanFormat

enum VulkanQueueFamily : uint32_t
{
    VulkanQueueFamilyUniversal = 0, // Default Family that supports all features
    VulkanQueueFamilyCompute,       // Async Compute Engine (ACE)
    VulkanQueueFamilyTransfer,      // DMA
    VulkanQueueFamilyCount
};

template<typename T, typename Next, typename... Rest>
void MakeVulkanStructureChain(T& t, Next& next, Rest&... rest)
{
    t.pNext = reinterpret_cast<void*>(&next);
    MakeVulkanStructureChain(next, rest...);
}

template<typename T, typename Next>
void MakeVulkanStructureChain(T& t, Next& next)
{
    t.pNext = &next;
    next.pNext = nullptr;
}

struct VkStructureHeader
{
    VkStructureType sType;
    void* pNext;
};

template<typename Head, typename Last>
void AppendVulkanStructureChain(Head& head, Last& last)
{
    VkStructureHeader* p = reinterpret_cast<VkStructureHeader*>(&head);
    while (p->pNext != nullptr)
    {
        p = reinterpret_cast<VkStructureHeader*>(p->pNext);
    }
    p->pNext = &last;
    last.pNext = nullptr;
}

struct ShaderMacro
{
    ShaderMacro()
    {
    }

    ShaderMacro(std::string_view name)
    {
        this->m_name = name;
    }

    ShaderMacro(std::string_view name, std::string_view definition)
    {
        this->m_name = name;
        this->m_definition = definition;
    }

    template<typename T>
    ShaderMacro(std::string_view name, T definition)
    {
        this->m_name = name;
        this->m_definition = std::to_string(definition);
    }

    std::string m_name;
    std::string m_definition;

}; // class ShaderMacro


struct VulkanBufferCreateInfo
{
    VkBufferCreateInfo m_createInfo;
    VmaAllocationCreateInfo m_allocation;

    VulkanBufferCreateInfo()
    {
        m_createInfo = {};
        m_createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        m_allocation = {};
        m_allocation.usage = VMA_MEMORY_USAGE_AUTO;
    }

    void SetUniformBuffer(VkDeviceSize size, bool isPersistentMapped)
    {
        m_createInfo.size = size;
        m_createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        m_allocation.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        if (isPersistentMapped)
        {
            m_allocation.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }
    }

    void SetStorageBuffer(VkDeviceSize size)
    {
        m_createInfo.size = size;
        m_createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        m_allocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    }

    void SetIndexBuffer(VkDeviceSize size)
    {
        m_createInfo.size = size;
        m_createInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        m_allocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    }

    void SetVertexBuffer(VkDeviceSize size)
    {
        m_createInfo.size = size;
        m_createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        m_allocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    }

    void SetStagingBuffer(VkDeviceSize size)
    {
        m_createInfo.size = size;
        m_createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        m_allocation.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    }

}; // struct VulkanBufferCreateInfo

uint32_t GetMaxMipLevel(uint32_t width, uint32_t height, uint32_t depth);

struct VulkanImageCreateInfo
{
    VkImageCreateInfo m_createInfo;
    VmaAllocationCreateInfo m_allocation;

    VulkanImageCreateInfo()
    {
        m_createInfo = {};
        m_createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        m_createInfo.pNext = nullptr;
        m_createInfo.flags = 0;
        m_createInfo.imageType = VK_IMAGE_TYPE_2D;
        m_createInfo.format = VK_FORMAT_UNDEFINED;
        m_createInfo.extent.width = 0;
        m_createInfo.extent.height = 0;
        m_createInfo.extent.depth = 1;
        m_createInfo.mipLevels = 1;
        m_createInfo.arrayLayers = 1;
        m_createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        m_createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        m_createInfo.usage = 0;
        m_createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        m_createInfo.queueFamilyIndexCount = 0;
        m_createInfo.pQueueFamilyIndices = nullptr;
        m_createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        m_allocation = {};
        m_allocation.usage = VMA_MEMORY_USAGE_AUTO;
    }

    VulkanImageCreateInfo& SetTexture2D(VkFormat format, uint32_t width, uint32_t height)
    {
        m_createInfo.imageType = VK_IMAGE_TYPE_2D;
        m_createInfo.format = format;
        m_createInfo.extent.width = width;
        m_createInfo.extent.height = height;
        m_createInfo.extent.depth = 1;
        m_createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        m_createInfo.usage =
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT;

        m_allocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        return *this;
    }

    VulkanImageCreateInfo& SetDepthStencil(VkFormat format, uint32_t width, uint32_t height)
    {
        m_createInfo.imageType = VK_IMAGE_TYPE_2D;
        m_createInfo.format = format;
        m_createInfo.extent.width = width;
        m_createInfo.extent.height = height;
        m_createInfo.extent.depth = 1;
        m_createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        m_createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        m_allocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        return *this;
    }

    VulkanImageCreateInfo& SetFullMiplevels()
    {
        m_createInfo.mipLevels =
            GetMaxMipLevel(m_createInfo.extent.width, m_createInfo.extent.height, m_createInfo.extent.depth);

        return *this;
    }

}; // struct VulkanImageCreateInfo

struct VulkanPipelineColorBlendAttachmentState
{
    VkPipelineColorBlendAttachmentState m_state;
    VulkanPipelineColorBlendAttachmentState()
    {
        m_state = {};
        m_state.blendEnable = VK_FALSE;
        m_state.srcColorBlendFactor;
        m_state.dstColorBlendFactor;
        m_state.colorBlendOp;
        m_state.srcAlphaBlendFactor;
        m_state.dstAlphaBlendFactor;
        m_state.alphaBlendOp;
        m_state.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
    }

    operator const VkPipelineColorBlendAttachmentState&()
    {
        return m_state;
    }

}; // struct VulkanPipelineColorBlendAttachmentState

struct VulkanGraphicsPipelineCreateInfo
{
    VulkanGraphicsPipelineCreateInfo();
    ~VulkanGraphicsPipelineCreateInfo();

    void AddVertexBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX)
    {
        VkVertexInputBindingDescription vertexBindingDesc = {};
        vertexBindingDesc.binding = binding;
        vertexBindingDesc.stride = stride;
        vertexBindingDesc.inputRate = inputRate;
        m_vertexInputState.bindings.push_back(vertexBindingDesc);
    }

    void AddVertexBinding(uint32_t binding, ArrayRef<VkFormat> attribFormats)
    {
        VkVertexInputBindingDescription vertexBindingDesc = {};
        vertexBindingDesc.binding = binding;
        vertexBindingDesc.stride = 0;
        vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        uint32_t location = 0;
        for (VkFormat format : attribFormats)
        {
            VkVertexInputAttributeDescription vertexAttribDesc = {};
            vertexAttribDesc.location = location;
            vertexAttribDesc.binding = binding;
            vertexAttribDesc.format = format;
            vertexAttribDesc.offset = vertexBindingDesc.stride;
            m_vertexInputState.attributes.push_back(vertexAttribDesc);

            location++;
            vertexBindingDesc.stride += VulkanFormat(format).GetElementSizeInBytes();
        }

        m_vertexInputState.bindings.push_back(vertexBindingDesc);
    }

    void AddVertexAttribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset)
    {
        VkVertexInputAttributeDescription vertexAttribDesc = {};
        vertexAttribDesc.location = location;
        vertexAttribDesc.binding = binding;
        vertexAttribDesc.format = format;
        vertexAttribDesc.offset = offset;
        m_vertexInputState.attributes.push_back(vertexAttribDesc);
    }

    std::vector<Ref<VulkanShader>> m_shaders;

    struct VertexInputState
    {
        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;
    } m_vertexInputState;

    struct InputAssemblyState
    {
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkBool32 primitiveRestartEnable = VK_FALSE;
    } m_inputAssemblyState;

    struct TessellationState
    {
        uint32_t patchControlPoints = 0;
    } m_tessellationState;

    // viewports and scissors should be set dynamically.
    uint32_t m_viewportCount = 1;
    uint32_t m_scissorCount = 1;

    struct RasterizationState
    {
        VkBool32                depthClampEnable = VK_FALSE;
        VkBool32                rasterizerDiscardEnable = VK_FALSE;
        VkPolygonMode           polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags         cullMode = VK_CULL_MODE_NONE;
        VkFrontFace             frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        VkBool32                depthBiasEnable = VK_FALSE;
        float                   depthBiasConstantFactor = 0;
        float                   depthBiasClamp = 0;
        float                   depthBiasSlopeFactor = 0;
        float                   lineWidth = 1.0f;
    } m_rasterizationState;

    struct MultisampleState
    {
        VkSampleCountFlagBits   rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        VkBool32                sampleShadingEnable = VK_FALSE;
        float                   minSampleShading = 0.0f;    // must be in the range [0,1]
        VkSampleMask            sampleMask = 0xFFFFFFFF;
        VkBool32                alphaToCoverageEnable = VK_FALSE;
        VkBool32                alphaToOneEnable = VK_FALSE;
    } m_multisampleState;

    struct DepthStencilState
    {
        VkBool32                depthTestEnable = VK_FALSE;
        VkBool32                depthWriteEnable = VK_FALSE;
        VkCompareOp             depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        VkBool32                depthBoundsTestEnable = VK_FALSE;
        VkBool32                stencilTestEnable = VK_FALSE;
        VkStencilOpState        front;
        VkStencilOpState        back;
        float                   minDepthBounds = 0.0f;
        float                   maxDepthBounds = 1.0f;
    } m_depthStencilState;

    struct ColorBlendState
    {
        VkBool32                logicOpEnable = VK_FALSE;
        VkLogicOp               logicOp;
        std::vector<VkPipelineColorBlendAttachmentState> attachments;
        float                   blendConstants[4];
    } m_colorBlendState;

    Ref<VulkanPipelineLayout>   m_layout;
    Ref<VulkanRenderPass>       m_renderPass;
    uint32_t                    m_subpass = 0;
    Ref<VulkanPipeline>         m_basePipeline;
    int32_t                     m_basePipelineIndex = 0;

}; // struct VulkanGraphicsPipelineCreateInfo

struct VulkanComputePipelineCreateInfo
{
    Ref<VulkanShader>           shader;
    Ref<VulkanPipelineLayout>   layout;
    Ref<VulkanPipeline>         basePipeline;
    int32_t                     basePipelineIndex = 0;

}; // struct VulkanComputePipelineCreateInfo


#endif // VULKAN_COMMON_H