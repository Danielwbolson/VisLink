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
				images = new EntityNode(graphicsObjects);
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
	Entity* images;

	std::map<std::string, Entity*> imageMap;
};




TextureManager::TextureManager() {
	state = new TextureManagerState();

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

void TextureManager::createSharedTexture(const std::string& name, const TextureInfo& info) {

	Entity* image = new EntityNode(state->images);
        image->addComponent(new Image(256, 256, 4));
        image->addComponent(new VulkanExternalImage());

	state->instanceNode.update();
	state->renderer->render(VULKAN_RENDER_UPDATE_SHARED);

    state->imageMap[name] = image;
	//externalHandle = 
}

Texture TextureManager::getSharedTexture(const std::string& name) {
	Entity* imageNode = state->imageMap[name];
	Image* image = imageNode->getComponent<Image>();
	Texture tex;
	tex.width = image->getWidth();
	tex.height = image->getHeight();
	tex.components = image->getComponents();
	tex.externalHandle = imageNode->getComponent<VulkanExternalImage>()->getExternalHandle(state->renderer->getContext());
	return tex;
}

}