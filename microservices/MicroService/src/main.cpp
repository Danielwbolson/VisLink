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
#include <thread>

#include <iostream>
#include <VisLink/net/Client.h>
#include <VisLink/impl/VisLinkAPIImpl.h> 
#include <VisLink/sync/SyncStrategy.h>
#include <src/services/ColorFilter.h>
#include <src/services/ThresholdFilter.h>

using namespace vislink;
using namespace std;

GLFWwindow* window;

void initGLFW() { 
	glfwInit(); 
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
	glfwWindowHint(GLFW_VISIBLE, false);

    window = glfwCreateWindow(512, 512, "OpenGL", nullptr, nullptr);

    glfwMakeContextCurrent(window);
    initializeGLExtentions();
    
}

int main(int argc, char**argv) {

	if (argc < 5) {
		std::cout << "Usage: ./build/bin/SimpleService <name> <type> <inputTex> <outputTex>" << std::endl;
		exit(0);
	}

	std::string serviceName = std::string(argv[1]);
	std::string type = std::string(argv[2]);
	std::string inputTexName = std::string(argv[3]);
	std::string outputTexName = std::string(argv[4]);

    initGLFW();

	vislink::Client* client = new vislink::Client();
	vislink::VisLinkAPI* api = client;
    api = new vislink::VisLinkOpenGL(api);

	MicroService* service = NULL;

	if (type == "ColorFilter") {
		service = new ColorFilter(serviceName, api);
	}	
	else if (type == "ThresholdFilter") {
		service = new ThresholdFilter(serviceName, api);
	}

	if (service) {
		service->addTexture(inputTexName, MICROSERVICE_INPUT_TEXTURE);
		service->addTexture(outputTexName, MICROSERVICE_OUTPUT_TEXTURE);

		while (true) {
			service->begin();
			service->end();
		}

		delete service;
	}

    glfwTerminate();

    delete api;

	return 0;
}


