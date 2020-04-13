//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
//#include "OpenGL.h"

#include "OpenGL.h"
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <VisLink/net/Server.h>
#include <VisLink/net/Client.h>
#include <VisLink/sync/SyncStrategy.h>

#define WIDTH 512
#define HEIGHT 512

#include "sandbox/image/Image.h"
#include<vector>

using namespace sandbox;


#if _MSC_VER // this is defined when compiling with Visual Studio

#define EXPORT_API __declspec(dllexport) // Visual Studio needs annotating exported functions with this

#else 

#define EXPORT_API // XCode does not need annotating exported functions, so define is empty

#endif

struct VisLinkContainer {
	VisLinkContainer() : api(NULL), isReady(false) {}
	vislink::VisLinkAPI* api;
	bool isReady;
};

struct TextureContainer {
	TextureContainer(int apiId, char* name, int deviceIndex) : name(name), deviceIndex(deviceIndex), apiId(apiId), isReady(false) {}
	std::string name;
	int deviceIndex;
	int apiId;
	vislink::Texture tex;
	bool isReady;
};

struct SemaphoreContainer {
	SemaphoreContainer(int apiId, char* name, int deviceIndex) : name(name), deviceIndex(deviceIndex), apiId(apiId), isReady(false) {
		//syncStrategy = new vislink::OpenGLSemaphoreSync();
		syncStrategy = new vislink::EmptySyncStrategy();
	}
	SemaphoreContainer() {
		delete syncStrategy;
	}
	vislink::SyncStrategy* syncStrategy;
	std::string name;
	int deviceIndex;
	int apiId;
	vislink::Semaphore sem;
	bool isReady;
};


std::vector<VisLinkContainer> apis;
std::vector<TextureContainer> textures;
std::vector<SemaphoreContainer> semaphores;

vislink::VisLinkAPI* getApi(int id) {
	return apis[id].api;
}

vislink::Texture& getTexture(int id) {
	return textures[id].tex;
}

vislink::Semaphore& getSemaphore(int id) {
	return semaphores[id].sem;
}

extern "C"
{
	// API methods
	EXPORT_API int createClientAPI(char* address, int port) {
		VisLinkContainer client;
		client.api = new vislink::Client(address, port);
		apis.push_back(client);
		return apis.size()-1;
	}

	EXPORT_API bool isAPIReady(int api) {
		return apis[api].isReady;
	}

	EXPORT_API void destroyAPI(int id) {
		//delete getApi(id);
	}

	EXPORT_API int getSharedTexture(int id, char* name, int deviceIndex) {
		TextureContainer tex(id, name, deviceIndex);
		textures.push_back(tex);
		return textures.size() - 1;
	}

	EXPORT_API bool isTextureReady(int textureId) {
		return textures[textureId].isReady;
	}

	EXPORT_API int getTextureWidth(int textureId) {
		return getTexture(textureId).width;
	}

	EXPORT_API int getTextureHeight(int textureId) {
		return getTexture(textureId).height;
	}

	EXPORT_API int getTextureId(int textureId) {
		return getTexture(textureId).id;
	}

	EXPORT_API int getSemaphore(int id, char* name, int deviceIndex) {
		SemaphoreContainer sem(id, name, deviceIndex);
		semaphores.push_back(sem);
		return semaphores.size() - 1;
	}

	EXPORT_API bool isSemaphoreReady(int semId) {
		return semaphores[semId].isReady;
	}

	EXPORT_API void semaphoreWriteTexture(int semId, int texId) {
		semaphores[semId].syncStrategy->addObject(vislink::WriteTexture(getTexture(texId)));
	}

	EXPORT_API void semaphoreReadTexture(int semId, int texId) {
		semaphores[semId].syncStrategy->addObject(vislink::ReadTexture(getTexture(texId)));
	}

	EXPORT_API void semaphoreSignal(int semId) {
		semaphores[semId].syncStrategy->signal();
	}

	EXPORT_API void semaphoreWaitForSignal(int semId) {
		semaphores[semId].syncStrategy->waitForSignal();
	}

	EXPORT_API void* getMessageQueue(int api, char* name) {
		vislink::MessageQueue* queue = getApi(api)->getMessageQueue(name);
		return queue;
	}

	EXPORT_API void waitForMessage(void* msgQueue) {
		vislink::MessageQueue* queue = static_cast<vislink::MessageQueue*>(msgQueue);
		queue->waitForMessage();
	}

	EXPORT_API void sendMessage(void* msgQueue) {
		vislink::MessageQueue* queue = static_cast<vislink::MessageQueue*>(msgQueue);
		queue->sendMessage();
	}

	EXPORT_API int queueRecieveInt(void* msgQueue) { 
		vislink::MessageQueue* queue = static_cast<vislink::MessageQueue*>(msgQueue);
		return queue->receiveObject<int>();  
	}

	EXPORT_API void queueRecieveFloatArray(void* msgQueue, float* arr, int size) {
		vislink::MessageQueue* queue = static_cast<vislink::MessageQueue*>(msgQueue);
		queue->receiveData((unsigned char*)arr, size * sizeof(float));
	}
}

#include "IUnityInterface.h"
#include "IUnityGraphics.h"

static IUnityInterfaces* s_UnityInterfaces = NULL;
static IUnityGraphics* s_Graphics = NULL;
static UnityGfxRenderer s_RendererType = kUnityGfxRendererNull;

static void UNITY_INTERFACE_API
OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	switch (eventType)
	{
	case kUnityGfxDeviceEventInitialize:
	{
		s_RendererType = s_Graphics->GetRenderer();

		if (s_RendererType == kUnityGfxRendererOpenGLCore) {
			initializeGLExtentions();
		}

		break;
	}
	case kUnityGfxDeviceEventShutdown:
	{
		s_RendererType = kUnityGfxRendererNull;
		break;
	}
	case kUnityGfxDeviceEventBeforeReset:
	{
		break;
	}
	case kUnityGfxDeviceEventAfterReset:
	{
		break;
	}
	};
}

// Unity plugin load event
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
	s_UnityInterfaces = unityInterfaces;
	s_Graphics = unityInterfaces->Get<IUnityGraphics>();

	s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

	// Run OnGraphicsDeviceEvent(initialize) manually on plugin load
	// to not miss the event in case the graphics device is already initialized
	OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

// Unity plugin unload event
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginUnload()
{
	s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

// Plugin function to handle a specific rendering event
static void UNITY_INTERFACE_API OnCreateTexture(int eventID)
{
	TextureContainer& textureContainer = textures[eventID];
	VisLinkContainer& apiContainer = apis[textureContainer.apiId];
	textureContainer.tex = apiContainer.api->getSharedTexture(textureContainer.name, textureContainer.deviceIndex);
	textureContainer.isReady = true;
}

// Freely defined function to pass a callback to plugin-specific scripts
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
GetCreateTextureFunc()
{
	return OnCreateTexture;
}

// Plugin function to handle a specific rendering event
static void UNITY_INTERFACE_API OnCreateAPI(int eventID)
{
	VisLinkContainer& apiContainer = apis[eventID];
	apiContainer.api = new vislink::VisLinkOpenGL(apiContainer.api);
	apiContainer.isReady = true;
}

// Freely defined function to pass a callback to plugin-specific scripts
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
GetCreateAPIFunc()
{
	return OnCreateAPI;
}

// Plugin function to handle a specific rendering event
static void UNITY_INTERFACE_API OnCreateSemaphore(int eventID)
{
	SemaphoreContainer& semaphoreContainer = semaphores[eventID];
	VisLinkContainer& apiContainer = apis[semaphoreContainer.apiId];
	semaphoreContainer.sem = apiContainer.api->getSemaphore(semaphoreContainer.name, semaphoreContainer.deviceIndex);
	semaphoreContainer.syncStrategy->addObject(semaphoreContainer.sem);
	semaphoreContainer.isReady = true;
}

// Freely defined function to pass a callback to plugin-specific scripts
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
GetCreateSemaphoreFunc()
{
	return OnCreateSemaphore;
}
