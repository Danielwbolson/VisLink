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
#include <VisLink/impl/VisLinkAPIImpl.h>

#define WIDTH 512
#define HEIGHT 512

using namespace vislink;


#if _MSC_VER // this is defined when compiling with Visual Studio

#define EXPORT_API __declspec(dllexport) // Visual Studio needs annotating exported functions with this

#else 

#define EXPORT_API // XCode does not need annotating exported functions, so define is empty

#endif


extern "C"
{
	// API methods
	EXPORT_API void* createClientAPI(char* address, int port, vislink::ProcLoader* procLoader) {
		//Server* server = new Server();
		//server->service();
		//return server;
		std::cout << procLoader << std::endl;
		return new VisLinkOpenGL(new Client(), procLoader);
		//return new Client();
		//return NULL;
	}

	EXPORT_API int createSharedTexture(void* api, char* name, int deviceIndex) {
		std::string texName(name);
		TextureInfo texInfo;
		texInfo.width = 1024;
		texInfo.height = 1024;
		texInfo.components = 4;
		static_cast<VisLinkAPI*>(api)->createSharedTexture(texName, texInfo, deviceIndex);
		return 1;
	}

	EXPORT_API int getSharedTexture(void* api, char* name, int deviceIndex) {
		std::string texName(name);
		std::cout << "getting" << std::endl;
		Texture tex = static_cast<VisLinkAPI*>(api)->getSharedTexture(texName, deviceIndex);
		std::cout << "setting" << std::endl;
		return tex.id;
	}

	EXPORT_API void test() {
		std::cout << "This is a test to see if it works." << std::endl;
	}
}