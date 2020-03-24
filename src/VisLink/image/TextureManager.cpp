#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "VisLink/image/TextureManager.h"
#include "sandbox/graphics/vulkan/VulkanDevice.h"
#include "sandbox/graphics/vulkan/VulkanQueue.h"
#include "sandbox/graphics/vulkan/VulkanDeviceRenderer.h"
#include "sandbox/graphics/vulkan/render/VulkanCommandPool.h"
#include "sandbox/graphics/vulkan/image/VulkanExternalImage.h"
#include "sandbox/graphics/vulkan/image/VulkanImportImage.h"
#include "sandbox/graphics/vulkan/sync/VulkanSemaphore.h"
#include "sandbox/image/Image.h"

#ifdef WIN32
#else
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
#endif

namespace vislink {

using namespace sandbox;

struct TextureManagerDeviceState {
	TextureManagerDeviceState(Entity& instanceNode, VkPhysicalDevice physicalDevice) {
		VulkanGraphicsQueue* graphicsQueue = new VulkanGraphicsQueue();

		deviceNode = new EntityNode(&instanceNode);
			deviceNode->addComponent(new VulkanDevice(&instanceNode, physicalDevice));
			deviceNode->addComponent(graphicsQueue);
			graphicsObjects = new EntityNode(deviceNode);
				images = new EntityNode(graphicsObjects);
			Entity* updateNode = new EntityNode(deviceNode);
				updateNode->addComponent(new VulkanCommandPool(graphicsQueue));
				renderer = new VulkanDeviceRenderer();
				updateNode->addComponent(renderer);
				updateNode->addComponent(new RenderNode(graphicsObjects));
	}

	Entity* deviceNode;
	Entity* graphicsObjects;
	VulkanDeviceRenderer* renderer;
	Entity* images;
	std::map<std::string, Entity*> imageMap;
};

class TextureManagerState {
public:
	TextureManagerState() {
		instanceNode.addComponent(new VulkanInstance());
		instanceNode.update();

		std::vector<VkPhysicalDevice> physicalDevices = instanceNode.getComponent<VulkanInstance>()->getPhysicalDevices();
		
		for (int f = 0; f < physicalDevices.size(); f++) {
			devices.push_back(TextureManagerDeviceState(instanceNode, physicalDevices[f]));
		}
	}

	TextureManagerDeviceState& getDeviceState(int index) {
		return devices[index % devices.size()];
	}

	EntityNode instanceNode;
	std::vector<TextureManagerDeviceState> devices;
};




TextureManager::TextureManager() {
	state = new TextureManagerState();

	state->instanceNode.update();

	for (int f = 0; f < state->devices.size(); f++) {
		std::cout << "Physical Device Id: " << state->devices[f].deviceNode->getComponent<VulkanDevice>()->getProperties().deviceID << " " << state->devices[f].deviceNode->getComponent<VulkanDevice>()->getProperties().deviceName << std::endl;
	}

	//createSharedTexture("test", TextureInfo());
	/*Entity* mainImage = new EntityNode(state->images);
        mainImage->addComponent(new Image(256, 256, 4));
        mainImage->addComponent(new VulkanExternalImage());


	state->instanceNode.update();
	state->renderer->render(VULKAN_RENDER_UPDATE_SHARED);
	externalHandle = mainImage->getComponent<VulkanExternalImage>()->getExternalHandle(state->renderer->getContext());
	//state->renderer->render(VULKAN_RENDER_UPDATE_SHARED);*/

}

TextureManager::~TextureManager() {
	delete state;
}

void TextureManager::createSharedTexture(const std::string& name, const TextureInfo& info, int deviceIndex) {

	Entity* image = new EntityNode(state->getDeviceState(deviceIndex).images);
        image->addComponent(new Image(info.width, info.height, info.components));
        image->addComponent(new VulkanExternalImage());
        for (int f = 0; f < NUM_TEXTURE_SEMAPHORES; f++) {
        	image->addComponent(new VulkanSemaphore(true));
        }

	state->instanceNode.update();
	state->getDeviceState(deviceIndex).renderer->render(VULKAN_RENDER_UPDATE_SHARED);
    state->getDeviceState(deviceIndex).imageMap[name] = image;
	//externalHandle = 

    std::vector<VulkanSemaphore*> semaphores = image->getComponents<VulkanSemaphore>();
    for (int f = 0; f < NUM_TEXTURE_SEMAPHORES; f++) {
        //tex.externalSemaphores[f] = semaphores[f]->getExternalHandle(); 
        std::cout << "" << " " << semaphores[f]->getExternalHandle() << std::endl;
    }
}

Texture TextureManager::getSharedTexture(const std::string& name, int deviceIndex) {
	Entity* imageNode = state->getDeviceState(deviceIndex).imageMap[name];
	Image* image = imageNode->getComponent<Image>();
	Texture tex;
	tex.width = image->getWidth();
	tex.height = image->getHeight();
	tex.components = image->getComponents();
	tex.externalHandle = imageNode->getComponent<VulkanExternalImage>()->getExternalHandle(state->getDeviceState(deviceIndex).renderer->getContext());
	tex.deviceIndex = deviceIndex;

	std::vector<VulkanSemaphore*> semaphores = imageNode->getComponents<VulkanSemaphore>();
	for (int f = 0; f < NUM_TEXTURE_SEMAPHORES; f++) {
		tex.externalSemaphores[f] = semaphores[f]->getExternalHandle();
		std::cout << tex.externalSemaphores[f] << " " << semaphores[f]->getExternalHandle() << std::endl;
	}

	return tex;
}

}
