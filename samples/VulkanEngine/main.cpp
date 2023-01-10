#include "rad/VulkanCore.h"

#define VK_ENGINE_VERSION_MAJOR 0
#define VK_ENGINE_VERSION_MINOR 0
#define VK_ENGINE_VERSION_PATCH 0
#define VK_ENGINE_VERSION VK_MAKE_VERSION(VK_ENGINE_VERSION_MAJOR, VK_ENGINE_VERSION_MINOR, VK_ENGINE_VERSION_PATCH)

int main(int argc, char* argv[])
{
    Application app(argc, argv);

    bool enableValidationLayer = false;
#ifdef _DEBUG
    enableValidationLayer = true;
#endif
    Ref<VulkanInstance> vulkanInstance = VulkanInstance::Create(
        "VulkanEngine", VK_ENGINE_VERSION,
        enableValidationLayer
    );

    return app.Run();
}
