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

std::vector<VisLinkContainer> apis;
std::vector<TextureContainer> textures;

vislink::VisLinkAPI* getApi(int id) {
	return apis[id].api;
}

vislink::Texture& getTexture(int id) {
	return textures[id].tex;
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
		/*vislink::VisLinkAPI* apiImpl = static_cast<vislink::VisLinkAPI*>(api);
		vislink::Texture* tex = new vislink::Texture();
		*tex = apiImpl->getSharedTexture(name, deviceIndex);
		return tex;*/
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

			/*vislink::Client* client = new vislink::Client();
			vislink::VisLinkAPI* api = client;
			api = new vislink::VisLinkOpenGL(api); 
			vislink::Texture tex = api->getSharedTexture("test.png");*/
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