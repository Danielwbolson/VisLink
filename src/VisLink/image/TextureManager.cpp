#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "VisLink/image/TextureManager.h"
#include "sandbox/graphics/vulkan/VulkanDevice.h"
#include "sandbox/graphics/vulkan/VulkanQueue.h"
#include "sandbox/graphics/vulkan/VulkanDeviceRenderer.h"
#include "sandbox/graphics/vulkan/render/VulkanCommandPool.h"
#include "sandbox/graphics/vulkan/image/VulkanExternalImage.h"
#include "sandbox/graphics/vulkan/image/VulkanImportImage.h"
#include "sandbox/image/Image.h"

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>        /* See NOTES */
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>

namespace vislink {

using namespace sandbox;

class TextureManagerState {
public:
	TextureManagerState() {
		VulkanGraphicsQueue* graphicsQueue = new VulkanGraphicsQueue();

		instanceNode.addComponent(new VulkanInstance());
		deviceNode = new EntityNode(&instanceNode);
			deviceNode->addComponent(new VulkanDevice(&instanceNode));
			deviceNode->addComponent(graphicsQueue);
			graphicsObjects = new EntityNode(deviceNode);
			Entity* updateNode = new EntityNode(deviceNode);
				updateNode->addComponent(new VulkanCommandPool(graphicsQueue));
				renderer = new VulkanDeviceRenderer();
				updateNode->addComponent(renderer);
				updateNode->addComponent(new RenderNode(graphicsObjects));
	}

	EntityNode instanceNode;
	Entity* deviceNode;
	Entity* graphicsObjects;
	VulkanDeviceRenderer* renderer;
};




TextureManager::TextureManager() {
	state = new TextureManagerState();

	//int new_fd = open("/proc/15844/fd/30", O_RDONLY);
    //std::cout << "again" << new_fd << std::endl;

	Entity* images = new EntityNode(state->graphicsObjects);
		Entity* mainImage = new EntityNode(images);
            mainImage->addComponent(new Image("../sandbox/examples/VulkanSandbox/textures/test.png"));
            mainImage->addComponent(new VulkanExternalImage());
            //mainImage->addComponent(new VulkanImportImage(new_fd));

	/*VkExternalMemoryHandleTypeFlagsNV handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
	VkExternalImageFormatPropertiesNV extProperties;
	VkResult res = vkGetPhysicalDeviceExternalImageFormatPropertiesNV(
	    state->deviceNode->getComponent<VulkanDevice>()->getPhysicalDevice(),
	    VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    handleType,
	    &extProperties);*/

	/*VkPhysicalDeviceMemoryProperties properties;
	vkGetPhysicalDeviceMemoryProperties(state->deviceNode->getComponent<VulkanDevice>()->getPhysicalDevice(), &properties);
    if ((properties.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) == 0) {
    	std::cout << "supported" << std::endl;
	}
	else {
    	std::cout << "not supported" << std::endl;
	}*/

	state->instanceNode.update();
	state->renderer->render(VULKAN_RENDER_UPDATE_SHARED);

    //int newFd = dup(29);
    //std::cout << newFd << std::endl;

    //int newFd = dup(29);
    //std::cout << newFd << std::endl;

                //int externalHandle =  mainImage2->getComponent<VulkanImage>()->getExternalHandle(sharedRenderer2->getContext());
                //std::cout << externalHandle << std::endl;
                /*GLuint mem = 0;
                glCreateMemoryObjectsEXT(1, &mem);
                glImportMemoryFdEXT(mem, mainImage2->getComponent<Image>()->getSize(), GL_HANDLE_TYPE_OPAQUE_FD_EXT, externalHandle);*/


	externalHandle = mainImage->getComponent<VulkanExternalImage>()->getExternalHandle(state->renderer->getContext());
}

TextureManager::~TextureManager() {
	delete state;
}

}