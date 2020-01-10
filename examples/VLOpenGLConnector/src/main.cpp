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

// Link following functions C-style (required for plugins)
static vislink::Texture tex;

extern "C"
{


	// The functions we will call from Unity.
	//
	const EXPORT_API char* PrintHello() {
		return "Hello\0";
	}



	int EXPORT_API PrintANumber() {
		//vislink::VisLinkAPI* api = new vislink::Client();
		//api = new vislink::VisLinkOpenGL(api);
		//vislink::Texture tex = api->getSharedTexture("test.png");
		/*//initGLFW();

		cout << "started..." << endl;


		vislink::VisLinkAPI* api = new vislink::Client();
		api = new vislink::VisLinkOpenGL(api);
		vislink::Texture tex = api->getSharedTexture("test.png");
		std::cout << tex.externalHandle << std::endl;
		std::cout << tex.id << std::endl;

		delete api;*/

		return tex.id;
	}



	int EXPORT_API AddTwoIntegers(int a, int b) {
		return a + b;
	}

	float EXPORT_API AddTwoFloats(float a, float b) {
		return a + b;
	}
} // end of export C block




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

		/*glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
		//glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

		GLFWwindow* tempWindow = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", nullptr, nullptr);
		glfwSetWindowPos(tempWindow, windowXPos, 0);

		glfwMakeContextCurrent(tempWindow);*/ 

		if (s_RendererType == kUnityGfxRendererOpenGLCore) {
			initializeGLExtentions();

			vislink::Client* client = new vislink::Client();
			vislink::VisLinkAPI* api = client;
			api = new vislink::VisLinkOpenGL(api); 
			tex = api->getSharedTexture("test.png");
			//client->sendData(client->socketFD, (unsigned char*)& tex.id, sizeof(int));
			/*
			vislink::VisLinkAPI* api = new vislink::VisLinkAPIImpl();
			api->createSharedTexture("test.png", vislink::TextureInfo());
			api = new vislink::VisLinkOpenGL(api);
			tex = api->getSharedTexture("test.png");
			//tex.id = 2;*/
		}
		//glfwMakeContextCurrent(NULL);
		//vislink::Texture tex = api->getSharedTexture("test.png");
		//glfwDestroyWindow(tempWindow);

		break;
	}
	case kUnityGfxDeviceEventShutdown:
	{
		s_RendererType = kUnityGfxRendererNull;
		//glfwTerminate();
		break;
	}
	case kUnityGfxDeviceEventBeforeReset:
	{
		//TODO: user Direct3D 9 code
		break;
	}
	case kUnityGfxDeviceEventAfterReset:
	{
		//TODO: user Direct3D 9 code
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
