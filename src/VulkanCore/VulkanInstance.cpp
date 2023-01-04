#include "rad/VulkanCore/VulkanInstance.h"
#include "rad/VulkanCore/VulkanPhysicalDevice.h"

#include "SDL2/SDL_vulkan.h"

VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

Ref<VulkanInstance> VulkanInstance::Create(const char* appName, uint32_t appVersion, bool enableValidationLayer)
{
    return new VulkanInstance(appName, appVersion, enableValidationLayer);
}

std::vector<VkLayerProperties> VulkanInstance::EnumerateInstanceLayers()
{
    // It's possible, though very rare, that the number of instance layers could change.
    // For example, installing something could include new layers that the loader would pick up -
    // between the initial query for the count and the request for VkLayerProperties.
    // The loader indicates that by returning a VK_INCOMPLETE status and will update the the count parameter.
    // The count parameter will be updated with the number of entries loaded into the data pointer -
    // in case the number of layers went down or is smaller than the size given.

    std::vector<VkLayerProperties> layers;
    uint32_t layerCount = 0;
    VkResult result = VK_SUCCESS;
    do {
        VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
        if (layerCount > 0)
        {
            layers.resize(layerCount);
            result = vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
        }
    } while (result == VK_INCOMPLETE);

    if (result < 0)
    {
        LogPrint("Vulkan", LogLevel::Warn, "vkEnumerateInstanceLayerProperties failed with %s", string_VkResult(result));
    }

    return layers;
}

std::vector<VkExtensionProperties> VulkanInstance::EnumerateInstanceExtensions(const char* layerName)
{
    std::vector<VkExtensionProperties> extensions;
    uint32_t extensionCount = 0;
    VkResult result = VK_SUCCESS;
    do {
        VK_CHECK(vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr));
        if (extensionCount > 0)
        {
            extensions.resize(extensionCount);
            result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, extensions.data());
        }

    } while (result == VK_INCOMPLETE);

    if (result < 0)
    {
        LogPrint("Vulkan", LogLevel::Warn, "vkEnumerateInstanceExtensionProperties failed with %s", string_VkResult(result));
    }

    return extensions;
}

bool HasExtension(ArrayRef<VkExtensionProperties> extensions, std::string_view extensionName)
{
    for (const VkExtensionProperties& extension : extensions)
    {
        if (StrEqual(extension.extensionName, extensionName))
        {
            return true;
        }
    }
    return false;
}

VulkanInstance::VulkanInstance(const char* appName, uint32_t appVersion, bool enableValidationLayer)
{
    if (SDL_Vulkan_LoadLibrary(nullptr) != 0)
    {
        LogPrint("Vulkan", LogLevel::Error, "SDL_Vulkan_LoadLibrary: %s", SDL_GetError());
        throw std::runtime_error("Cannot load Vulkan loader library!");
    }

    VK_CHECK(volkInitialize());

    if (vkEnumerateInstanceVersion)
    {
        vkEnumerateInstanceVersion(&m_apiVersion);
        LogPrint("Vulkan", LogLevel::Info, "Vulkan instance version: %s", GetVulkanVersionString(m_apiVersion).c_str());
    }

    std::vector<const char*> enabledLayerNames;

    std::vector<VkLayerProperties> layers = EnumerateInstanceLayers();
    if (enableValidationLayer)
    {
        const char* pValidationLayerName = "VK_LAYER_KHRONOS_validation";
        bool validationLayerFound = false;
        for (const VkLayerProperties& layer : layers)
        {
            if (StrEqual(layer.layerName, pValidationLayerName))
            {
                enabledLayerNames.push_back(pValidationLayerName);
                validationLayerFound = true;
            }
        }

        if (!validationLayerFound)
        {
            LogPrint("Vulkan", LogLevel::Warn, "Failed to find validation layer %s", pValidationLayerName);
            enableValidationLayer = false;
        }
    }

    std::vector<const char*> sdlExtensions;
    unsigned int extensionCount = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(NULL, &extensionCount, NULL))
    {
        LogPrint("Vulkan", LogLevel::Error, "SDL_Vulkan_GetInstanceExtensions failed");
    }
    sdlExtensions.resize(extensionCount);
    if (!SDL_Vulkan_GetInstanceExtensions(NULL, &extensionCount, sdlExtensions.data()))
    {
        LogPrint("Vulkan", LogLevel::Error, "SDL_Vulkan_GetInstanceExtensions failed");
    }

    std::vector<VkExtensionProperties> extensions = EnumerateInstanceExtensions(nullptr);

    for (const char* sdlExtension : sdlExtensions)
    {
        if (HasExtension(extensions, sdlExtension))
        {
            m_enabledExtensionNames.push_back(sdlExtension);
        }
        else
        {
            LogPrint("Vulkan", LogLevel::Warn, "Failed to find SDL extension %s", sdlExtension);
        }
    }

    if (HasExtension(extensions, "VK_KHR_get_physical_device_properties2"))
    {
        m_enabledExtensionNames.push_back("VK_KHR_get_physical_device_properties2");
    }
    if (HasExtension(extensions, "VK_KHR_get_surface_capabilities2"))
    {
        m_enabledExtensionNames.push_back("VK_KHR_get_surface_capabilities2");
    }

    if (enableValidationLayer)
    {
        if (HasExtension(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
        {
            m_enabledExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else
        {
            LogPrint("Vulkan", LogLevel::Warn, "Cannot enable validation layer: failed to find instance extension %s", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            enableValidationLayer = false;
        }
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = appVersion;
    appInfo.pEngineName = appName;
    appInfo.engineVersion = appVersion;
    appInfo.apiVersion = m_apiVersion;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.flags = 0;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayerNames.size());
    instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_enabledExtensionNames.size());
    instanceCreateInfo.ppEnabledExtensionNames = m_enabledExtensionNames.data();

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};
    if (enableValidationLayer)
    {
        debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugMessengerCreateInfo.pNext = nullptr;
        debugMessengerCreateInfo.flags = 0;
        debugMessengerCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugMessengerCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugMessengerCreateInfo.pfnUserCallback = DebugUtilsMessengerCallback;
        debugMessengerCreateInfo.pUserData = nullptr;

        AppendVulkanStructureChain(instanceCreateInfo, debugMessengerCreateInfo);
    }

    VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &m_handle));
    if (m_handle)
    {
        volkLoadInstance(m_handle);
    }

    if (enableValidationLayer)
    {
        VK_CHECK(vkCreateDebugUtilsMessengerEXT(m_handle, &debugMessengerCreateInfo, nullptr, &m_debugMessenger));
    }
}

VulkanInstance::~VulkanInstance()
{
    if (m_debugMessenger != VK_NULL_HANDLE)
    {
        vkDestroyDebugUtilsMessengerEXT(m_handle, m_debugMessenger, nullptr);
        m_debugMessenger = VK_NULL_HANDLE;
    }

    vkDestroyInstance(m_handle, nullptr);

    SDL_Vulkan_UnloadLibrary();
}

bool VulkanInstance::SupportsExtension(std::string_view extensionName)
{
    for (const char* enabledExtensionName : m_enabledExtensionNames)
    {
        if (StrEqual(extensionName, enabledExtensionName))
        {
            return true;
        }
    }
    return false;
}

std::vector<Ref<VulkanPhysicalDevice>> VulkanInstance::EnumeratePhysicalDevices()
{
    std::vector<Ref<VulkanPhysicalDevice>> physicalDevices;
    uint32_t physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(GetHandle(), &physicalDeviceCount, nullptr));
    if (physicalDeviceCount > 0)
    {
        std::vector<VkPhysicalDevice> physicalDevicesHandles(physicalDeviceCount);
        VK_CHECK(vkEnumeratePhysicalDevices(GetHandle(), &physicalDeviceCount, physicalDevicesHandles.data()));
        physicalDevices.resize(physicalDevicesHandles.size());
        for (int i = 0; i < physicalDevicesHandles.size(); i++)
        {
            physicalDevices[i] = new VulkanPhysicalDevice(this, physicalDevicesHandles[i]);
        }
    }
    return physicalDevices;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:  LogPrint("Vulkan", LogLevel::Verbose,    "[%s] %s", pCallbackData->pMessageIdName, pCallbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:     LogPrint("Vulkan", LogLevel::Info,       "[%s] %s", pCallbackData->pMessageIdName, pCallbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:  LogPrint("Vulkan", LogLevel::Warn,       "[%s] %s", pCallbackData->pMessageIdName, pCallbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:    LogPrint("Vulkan", LogLevel::Error,      "[%s] %s", pCallbackData->pMessageIdName, pCallbackData->pMessage); break;
    }

    return VK_FALSE;
}
