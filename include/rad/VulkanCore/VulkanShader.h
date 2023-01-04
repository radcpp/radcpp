#ifndef VULKAN_SHADER_H
#define VULKAN_SHADER_H
#pragma once

#include "VulkanCommon.h"

class VulkanShaderPrivate;

enum class ShaderLanguage
{
    GLSL,
    HLSL,
};

class VulkanShader : public VulkanObject
{
public:
    VulkanShader();
    VulkanShader(VkShaderStageFlagBits stage);
    ~VulkanShader();

    VulkanShader& SetStage(VkShaderStageFlagBits stage);
    VulkanShader& SetEntryPoint(const std::string_view entryPoint);
    VulkanShader& SetMacros(ArrayRef<ShaderMacro> macros);
    VulkanShader& SetLanguage(ShaderLanguage language);
    VulkanShader& SetIncludeDir(const std::string_view includeDir);
    VulkanShader& SetBinary(const ArrayRef<uint32_t> binary);

    VkShaderStageFlagBits GetStage() const;
    std::string GetFileName() const;
    const char* GetEntryPoint() const;
    const std::vector<ShaderMacro>& GetMacros() const;
    ShaderLanguage GetLanguage() const;

    bool Compile(const std::string_view fileName, const std::string_view source, const std::string_view entryPoint, ArrayRef<ShaderMacro> macros);
    bool Compile(const std::string_view fileName, const std::string_view source);
    bool LoadBinaryFromFile(const std::string_view fileName);
    bool IsValid() const;

    const std::vector<uint32_t>& GetBinary() const;
    const char* GetLog() const;

private:
    Ref<VulkanShaderPrivate> d_ptr;

}; // class VulkanShader

class VulkanShaderModule : public VulkanObject
{
public:
    VulkanShaderModule(Ref<VulkanDevice> device, const VkShaderModuleCreateInfo& createInfo);
    ~VulkanShaderModule();

    VkShaderModule GetHandle() const { return m_handle; }

private:
    Ref<VulkanDevice> m_device;
    VkShaderModule m_handle;

}; // class VulkanShaderModule

#endif // VULKAN_SHADER_H