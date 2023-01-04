#include "HelloWorld.h"
#include "rad/Core/Logging.h"

#include "rad/VulkanCore.h"

int main(int argc, char* argv[])
{
    Application app(argc, argv);

    Ref<VulkanInstance> vulkanInstance = VulkanInstance::Create(
        "HelloWorld", 0, true
    );

    return app.Run();
}
