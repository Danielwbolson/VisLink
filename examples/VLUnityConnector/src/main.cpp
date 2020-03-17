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

using namespace sandbox;


#if _MSC_VER // this is defined when compiling with Visual Studio

#define EXPORT_API __declspec(dllexport) // Visual Studio needs annotating exported functions with this

#else 

#define EXPORT_API // XCode does not need annotating exported functions, so define is empty

#endif

extern "C"
{
	// API methods
	EXPORT_API void* createClientAPI() {
		return new vislink::VisLinkOpenGL(new vislink::Client());
	}

	EXPORT_API void destroyAPI(void* api) {
		delete static_cast<vislink::VisLinkAPI*>(api);
	}

	EXPORT_API void* getSharedTexture(void* api, char* name, int deviceIndex) {
		vislink::VisLinkAPI* apiImpl = static_cast<vislink::VisLinkAPI*>(api);
		vislink::Texture* tex = new vislink::Texture();
		*tex = apiImpl->getSharedTexture(name, deviceIndex);
		return tex;
	}

	EXPORT_API void releaseTexture(void* tex) {
		delete static_cast<vislink::Texture*>(tex);
	}

	EXPORT_API int getTextureWidth(void* tex) {
		vislink::Texture* texture = static_cast<vislink::Texture*>(tex);
		return texture->width;
	}

	EXPORT_API int getTextureHeight(void* tex) {
		vislink::Texture* texture = static_cast<vislink::Texture*>(tex);
		return texture->height;
	}

	EXPORT_API int getTextureId(void* tex) {
		vislink::Texture* texture = static_cast<vislink::Texture*>(tex);
		return texture->id;
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
			tex = api->getSharedTexture("test.png");*/
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
static void UNITY_INTERFACE_API OnCreateTextures(int eventID)
{
}

// Freely defined function to pass a callback to plugin-specific scripts
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
GetCreateTexturesFunc()
{
	return OnCreateTextures;
}