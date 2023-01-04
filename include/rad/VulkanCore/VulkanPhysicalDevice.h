#ifndef VULKAN_PHYSICAL_DEVICE_H
#define VULKAN_PHYSICAL_DEVICE_H
#pragma once

#include "VulkanCommon.h"

class VulkanPhysicalDevice : public VulkanObject
{
public:
    VulkanPhysicalDevice(Ref<VulkanInstance> instance, VkPhysicalDevice handle);
    ~VulkanPhysicalDevice();

    VkPhysicalDevice GetHandle() const { return m_handle; }

    const VkPhysicalDeviceFeatures& GetFeatures() const { return m_features; }
    const VkPhysicalDeviceProperties& GetProperties() const { return m_properties; }

    const VkPhysicalDeviceFeatures2& GetFeatures2() const { return m_features2; }
    const VkPhysicalDeviceProperties2& GetProperties2() const { return m_properties2; }
    const VkPhysicalDeviceVulkan11Features& GetVulkan11Features() const { return m_vulkan11Features; }
    const VkPhysicalDeviceVulkan12Features& GetVulkan12Features() const { return m_vulkan12Features; }
    const VkPhysicalDeviceRayTracingPipelineFeaturesKHR& GetRayTracingPipelineFeatures() const { return m_rayTracingPipelineFeatures; }
    const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& GetRayTracingPipelineProperties() const { return m_rayTracingPipelineProperties; }
    const char* GetDeviceName() const { return m_properties.deviceName; }
    const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return m_memoryProperties; }
    const std::vector<VkQueueFamilyProperties>& GetQueueFamilyProperties() const { return m_queueFamilyProperties; }
    const std::vector<VkExtensionProperties>& GetDeviceExtensionProperties() const { return m_extensionProperties; }
    bool SupportsDeviceExtension(std::string_view extensionName) const;

    VkFormatProperties GetFormatProperties(VkFormat format) const;
    VkFormat FindFormat(ArrayRef<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    bool SupportsSurface(uint32_t queueFamilyIndex, VkSurfaceKHR surface) const;
    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(VkSurfaceKHR surface);
    std::vector<VkPresentModeKHR> GetSurfacePresentModes(VkSurfaceKHR surface);
    std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(VkSurfaceKHR surface);

    Ref<VulkanDevice> CreateDevice(const std::vector<std::string>& extensionNames);
    Ref<VulkanDevice> CreateDevice();

private:
    void EnumerateDeviceExtensions();

    Ref<VulkanInstance>                         m_instance;
    VkPhysicalDevice                            m_handle;
    VkPhysicalDeviceFeatures                    m_features;
    VkPhysicalDeviceProperties                  m_properties;
    VkPhysicalDeviceMemoryProperties            m_memoryProperties;
    std::vector<VkQueueFamilyProperties>        m_queueFamilyProperties;
    std::vector<VkExtensionProperties>          m_extensionProperties;

    void QueryFeaturesAndProperties2();
    VkPhysicalDeviceFeatures2 m_features2;
    VkPhysicalDeviceProperties2 m_properties2;
    VkPhysicalDeviceVulkan11Features m_vulkan11Features;
    VkPhysicalDeviceVulkan12Features m_vulkan12Features;
    VkPhysicalDeviceSynchronization2FeaturesKHR m_synchronization2Features;
    VkPhysicalDeviceAccelerationStructureFeaturesKHR m_accelerationStructureFeatures;
    VkPhysicalDeviceAccelerationStructurePropertiesKHR m_accelerationStructureProperties;
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR m_rayTracingPipelineFeatures;
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rayTracingPipelineProperties;

}; // class VulkanPhysicalDevice

#endif // VULKAN_PHYSICAL_DEVICE_H
