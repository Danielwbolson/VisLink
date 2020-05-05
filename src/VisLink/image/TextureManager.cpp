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
			semaphores = new EntityNode(deviceNode);
	}

	Entity* deviceNode;
	Entity* graphicsObjects;
	VulkanDeviceRenderer* renderer;
	Entity* images;
	Entity* semaphores;
	std::map<std::string, Entity*> imageMap;
	std::map<std::string, TextureInfo> imageInfo;
	std::map<std::string, unsigned int> imageId;
	std::map<std::string, Entity*> semaphoreMap;
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

	std::map<std::string,sandbox::Entity*>::iterator it = state->getDeviceState(deviceIndex).imageMap.find(name);
	if (it != state->getDeviceState(deviceIndex).imageMap.end()) {
		return;
	}

	VkFormat format = VK_FORMAT_UNDEFINED;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	std::cout << "Created format: " << info.format << std::endl;

	switch(info.format) {
		case TEXTURE_FORMAT_RGBA8_UNORM:
			format = VK_FORMAT_R8G8B8A8_UNORM;
			break;
		case TEXTURE_FORMAT_RGBA16_UNORM:
			format = VK_FORMAT_R16G16B16A16_UNORM;
			break;
		case TEXTURE_FORMAT_RGBA32_UINT:
			format = VK_FORMAT_R32G32B32A32_UINT;
			break;
		case TEXTURE_FORMAT_DEPTH32F:
			format = VK_FORMAT_D32_SFLOAT;
			usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			break;
	};

	Entity* image = new EntityNode(state->getDeviceState(deviceIndex).images);
        image->addComponent(new Image(info.width, info.height, info.components));
        image->addComponent(new VulkanExternalImage(true, format, usage));

	state->instanceNode.update();
	state->getDeviceState(deviceIndex).renderer->render(VULKAN_RENDER_UPDATE_SHARED);
    state->getDeviceState(deviceIndex).imageMap[name] = image;
    state->getDeviceState(deviceIndex).imageInfo[name] = info;

	static unsigned int numImages = 0;
	state->getDeviceState(deviceIndex).imageId[name] = numImages;
	numImages++;
    
	//externalHandle = 

    std::vector<VulkanSemaphore*> semaphores = image->getComponents<VulkanSemaphore>();
}

Texture TextureManager::getSharedTexture(const std::string& name, int deviceIndex) {
	Entity* imageNode = state->getDeviceState(deviceIndex).imageMap[name];
	Image* image = imageNode->getComponent<Image>();
	TextureInfo info = state->getDeviceState(deviceIndex).imageInfo[name];
	Texture tex;
	tex.width = info.width;
	tex.height = info.height;
	tex.components = info.components;
	tex.format = info.format;
	tex.externalHandle = imageNode->getComponent<VulkanExternalImage>()->getExternalHandle(state->getDeviceState(deviceIndex).renderer->getContext());
	tex.deviceIndex = deviceIndex;
	tex.visLinkId = state->getDeviceState(deviceIndex).imageId[name];

	return tex;
}

Semaphore TextureManager::getSemaphore(const std::string& name, int deviceIndex) {
	std::map<std::string, Entity*>::iterator it = state->getDeviceState(deviceIndex).semaphoreMap.find(name);
	Entity* semaphoreNode = NULL;
	if (it != state->getDeviceState(deviceIndex).semaphoreMap.end()) {
		semaphoreNode = it->second;
	}
	else {
		semaphoreNode = new EntityNode(state->getDeviceState(deviceIndex).semaphores);
        	semaphoreNode->addComponent(new VulkanSemaphore(true));
        	
		state->instanceNode.update();
		state->getDeviceState(deviceIndex).semaphoreMap[name] = semaphoreNode;
	}

	Semaphore semaphore;
	semaphore.externalHandle = semaphoreNode->getComponent<VulkanSemaphore>()->getExternalHandle();
	semaphore.deviceIndex = deviceIndex;
	return semaphore;
}

}
