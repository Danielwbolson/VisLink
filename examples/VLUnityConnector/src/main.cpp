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
#include <sstream>

#define WIDTH 512
#define HEIGHT 512

#include "sandbox/image/Image.h"
#include<vector>
#include<thread>

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
		syncStrategy = new vislink::OpenGLSemaphoreSync();
		//syncStrategy = new vislink::EmptySyncStrategy();
	}
	SemaphoreContainer() {
		delete syncStrategy;
	}
	vislink::SyncStrategy* syncStrategy;
	std::string name;
	int deviceIndex;
	int apiId;
	vislink::Semaphore sem;
	std::vector<int> writeTextures;
	std::vector<int> readTextures;
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

std::thread::id firstThreadId;
std::thread::id secondThreadId;
std::thread::id renderThreadId;
bool semaphoreExists;

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

	EXPORT_API int getThreadId(char* str) {
		std::stringstream ss;
		ss << firstThreadId << " " << secondThreadId << " " << renderThreadId << semaphoreExists;
		std::string s = ss.str();
		strcpy(str, s.c_str());
		return s.size();
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

	EXPORT_API int getSemaphoreId(int semId) { 
		return semaphores[semId].sem.id + static_cast<vislink::OpenGLSemaphoreSync*>(semaphores[semId].syncStrategy)->semaphore.id*100 + static_cast<vislink::OpenGLSemaphoreSync*>(semaphores[semId].syncStrategy)->semaphore.id*10000;
	}

	EXPORT_API void semaphoreWriteTexture(int semId, int texId) {
		//semaphores[semId].syncStrategy->addObject(vislink::WriteTexture(getTexture(texId)));
		semaphores[semId].writeTextures.push_back(texId);
	}

	EXPORT_API void semaphoreReadTexture(int semId, int texId) {
		//semaphores[semId].syncStrategy->addObject(vislink::ReadTexture(getTexture(texId)));
		semaphores[semId].readTextures.push_back(texId);
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

		if (s_RendererType == kUnityGfxRendererOpenGLCore) {
			textures.clear();
			semaphores.clear();
			apis.clear();
		}

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

	if (!semaphoreContainer.isReady) {
		firstThreadId = std::this_thread::get_id();
	}
	else {
		secondThreadId = std::this_thread::get_id();
	}

		VisLinkContainer& apiContainer = apis[semaphoreContainer.apiId];
		semaphoreContainer.sem = apiContainer.api->getSemaphore(semaphoreContainer.name, semaphoreContainer.deviceIndex);
		semaphoreContainer.syncStrategy->addObject(semaphoreContainer.sem);

		for (int f = 0; f < semaphoreContainer.writeTextures.size(); f++) {
			semaphoreContainer.syncStrategy->addObject(vislink::WriteTexture(getTexture(semaphoreContainer.writeTextures[f])));
		}

		for (int f = 0; f < semaphoreContainer.readTextures.size(); f++) {
			semaphoreContainer.syncStrategy->addObject(vislink::ReadTexture(getTexture(semaphoreContainer.readTextures[f])));
		}

	semaphoreContainer.isReady = true;
}

// Freely defined function to pass a callback to plugin-specific scripts
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
GetCreateSemaphoreFunc()
{
	return OnCreateSemaphore; 
}

// Plugin function to handle a specific rendering event
static void UNITY_INTERFACE_API OnSemaphoreWaitForSignal(int eventID)
{
	//OnCreateSemaphore(eventID);	
	semaphoreExists = vislink::isSemaphore(static_cast<vislink::OpenGLSemaphoreSync*>(semaphores[eventID].syncStrategy)->semaphore.id);

	if (!semaphoreExists) {
		OnCreateSemaphore(eventID);
	}

	semaphores[eventID].syncStrategy->waitForSignal();
}

// Freely defined function to pass a callback to plugin-specific scripts
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
GetSemaphoreWaitForSignalFunc()
{
	return OnSemaphoreWaitForSignal;
}

// Plugin function to handle a specific rendering event
static void UNITY_INTERFACE_API OnSemaphoreSignal(int eventID)
{
	renderThreadId = std::this_thread::get_id();
	semaphoreExists = vislink::isSemaphore(static_cast<vislink::OpenGLSemaphoreSync*>(semaphores[eventID].syncStrategy)->semaphore.id);

	if (!semaphoreExists) {
		OnCreateSemaphore(eventID);
	}

	//OnCreateSemaphore(eventID);
	semaphores[eventID].syncStrategy->signal();
}

// Freely defined function to pass a callback to plugin-specific scripts
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API 
GetSemaphoreSignalFunc()
{
	return OnSemaphoreSignal; 
}
