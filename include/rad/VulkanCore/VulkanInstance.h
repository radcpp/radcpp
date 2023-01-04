#ifndef VULKAN_INSTANCE_H
#define VULKAN_INSTANCE_H
#pragma once

#include "VulkanCommon.h"

class VulkanInstance : public VulkanObject
{
public:
    static Ref<VulkanInstance> Create(const char* appName, uint32_t appVersion, bool enableValidationLayer = false);

    VulkanInstance(const char* appName, uint32_t appVersion, bool enableValidationLayer);
    ~VulkanInstance();

    VkInstance GetHandle() const { return m_handle; }
    uint32_t GetApiVersion() const { return m_apiVersion; }

    bool SupportsExtension(std::string_view extensionName);

    std::vector<Ref<VulkanPhysicalDevice>> EnumeratePhysicalDevices();

private:
    static std::vector<VkLayerProperties> EnumerateInstanceLayers();
    static std::vector<VkExtensionProperties> EnumerateInstanceExtensions(const char* layerName);

    VkInstance m_handle = VK_NULL_HANDLE;
    uint32_t m_apiVersion = 0;
    std::vector<const char*> m_enabledExtensionNames;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

}; // class VulkanInstance

#endif // VULKAN_INSTANCE_H