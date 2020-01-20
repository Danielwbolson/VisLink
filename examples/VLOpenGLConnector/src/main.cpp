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

#include <thread>
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

using namespace sandbox;


#if _MSC_VER // this is defined when compiling with Visual Studio

#define EXPORT_API __declspec(dllexport) // Visual Studio needs annotating exported functions with this

#else 

#define EXPORT_API // XCode does not need annotating exported functions, so define is empty

#endif

// Link following functions C-style (required for plugins)
//static vislink::Texture tex;

namespace vislink {

	struct DisplayInfo {
		DisplayInfo(std::string name, int width, int height, int xPos, int yPos, int device = 0) : name(name), width(width), height(height), xPos(xPos), yPos(yPos), device(device) {}
		std::string name;
		int width;
		int height;
		int xPos;
		int yPos;
		int device;
	};

	class DisplayManager
	{
	public:
		DisplayManager() : api(NULL), thread(NULL) {
			api = new vislink::VisLinkAPIImpl();
			/*api->createSharedTexture("leftWall_l", TextureInfo(), 0);
			api->createSharedTexture("leftWall_r", TextureInfo(), 0);
			api->createSharedTexture("frontWall", TextureInfo(), 0);
			api->createSharedTexture("rightWall", TextureInfo(), 1);
			api->createSharedTexture("floor", TextureInfo(), 1);*/
			running = false;
			readyToRender = false;
			finishedRendering = false;
			frame = 0;
		}

		~DisplayManager() {
			running = false;
			if (thread) {
				sync();
				thread->join();
			}
			delete thread;
			delete api;
		}

		void createDisplay(DisplayInfo displayInfo) {
			api->createSharedTexture(displayInfo.name + "_l", TextureInfo(), displayInfo.device);
			api->createSharedTexture(displayInfo.name + "_r", TextureInfo(), displayInfo.device);
			displayInfos.push_back(displayInfo);
		}

		void createContextTextures() {
			//initializeGLExtentions();
			for (int f = 0; f < displayInfos.size(); f++) {
				//contextTextures.push_back(NULL);
				//Texture tex = api->getSharedTexture(displayInfos[f].name + "_l");
				//contextTextures.push_back(createOpenGLTexture));
				contextTextures.push_back(createOpenGLTexture(api->getSharedTexture(displayInfos[f].name + "_l")));
				contextTextures.push_back(createOpenGLTexture(api->getSharedTexture(displayInfos[f].name + "_r")));
			}
		}

		void run()
		{
			running = true;
			std::cout << " api " << api << std::endl;
			glfwInit();

			std::vector<vislink::Display*> displays;
			for (int f = 0; f < displayInfos.size(); f++) {
				DisplayInfo displayInfo = displayInfos[f];
				displays.push_back(new TextureDisplay(api->getSharedTexture(displayInfo.name + "_l"), api->getSharedTexture(displayInfo.name + "_r"), displayInfo.width, displayInfo.height, displayInfo.xPos, displayInfo.yPos));
			}
			//displays.push_back(new TextureDisplay(api->getSharedTexture("leftWall_l"), api->getSharedTexture("leftWall_r"), 256, 256, 0, 100));
			/*displays.push_back(new TextureDisplay(api->getSharedTexture("leftWall_l"), api->getSharedTexture("leftWall_r"), 256, 256, 0, 100));
			displays.push_back(new TextureDisplay(api->getSharedTexture("frontWall"), 256, 256, 256, 100));
			displays.push_back(new TextureDisplay(api->getSharedTexture("rightWall"), 256, 256, 512, 100));
			displays.push_back(new TextureDisplay(api->getSharedTexture("floor"), 256, 256, 256, 356));*/

			while (running) {
				//glfwPollEvents();
				{
					std::unique_lock<std::mutex> lk(mtx);
					cv.wait(lk);

						//for (int f = displays.size() - 1; f >= 0; f--) {
						for (int f = 0; f < displays.size(); f++) {
						//for (int f = 0; f < 1; f++) {
							displays[f]->display();
						}
					
						//for (int f = displays.size() - 1; f >= 0; f--) {
						for (int f = 0; f < displays.size(); f++) {
						//for (int f = 0; f < 2; f++) {
							displays[f]->render();
						}

						//for (int f = displays.size() - 1; f >= 0; f--) {
						for (int f = 0; f < displays.size(); f++) {
						//for (int f = 0; f < 2; f++) {
							displays[f]->finish();
						}


					/*{
						std::lock_guard<std::mutex> lk(mtx);
						cv.notify_one(); 
					}*/
						lk.unlock();

					finishedRendering = true;
					//cv.notify_one();
					frame++;
				}
			}
			 
			for (int f = 0; f < displays.size(); f++) {
				delete displays[f];
			}

			glfwTerminate();
		}

		void start() {
			thread = new std::thread(&DisplayManager::run, this);
		}

		std::vector<OpenGLTexture*> contextTextures;

		void sync() {

				std::lock_guard<std::mutex> lk(mtx);
				cv.notify_one();
			/*{
				std::unique_lock<std::mutex> lk(mtx);
				cv.wait(lk);
			}*/
			/*{
				std::lock_guard<std::mutex> lk(mtx);
				readyToRender = true;
				finishedRendering = false;
			}
			cv.notify_one();
			{
				std::unique_lock<std::mutex> lk(mtx);
				cv.wait(lk, [this] {return finishedRendering; });
				readyToRender = false;
				finishedRendering = false;
			}*/

		}
		int frame;

	private:
		bool running;
		VisLinkAPI* api;
		std::thread* thread;
		std::vector<DisplayInfo> displayInfos;
		std::mutex mtx;
		std::condition_variable cv;
		bool readyToRender;
		bool finishedRendering;

	};
}

static vislink::DisplayManager* currentDisplayManager = nullptr;

extern "C"
{
	EXPORT_API void* createDisplayManager() {
		vislink::DisplayManager* dm = new vislink::DisplayManager();
		currentDisplayManager = dm;
		//dm->start();
		return dm;
		//std::thread* th = new std::thread(&vislink::DisplayManager::run, dm); // Pass 10 to member function
	}

	EXPORT_API void createDisplay(void* displayManager, char* name, int width, int height, int xPos, int yPos, int device) {
		vislink::DisplayManager* dm = static_cast<vislink::DisplayManager*>(displayManager);
		dm->createDisplay(vislink::DisplayInfo(name, width, height, xPos, yPos, device));
	}

	EXPORT_API void startDisplay(void* displayManager) {
		vislink::DisplayManager* dm = static_cast<vislink::DisplayManager*>(displayManager);
		dm->start();
	}

	EXPORT_API void destroyDisplayManager(void* displayManager) {
		delete static_cast<vislink::DisplayManager*>(displayManager);
		currentDisplayManager = nullptr;
	}

	EXPORT_API int getTexture(void* displayManager, int index) {
		//currentDisplayManager->createContextTextures();
		vislink::DisplayManager* dm = static_cast<vislink::DisplayManager*>(displayManager);
		return dm->contextTextures[index]->getId();
	}

	EXPORT_API int syncFrame(void* displayManager) {
		vislink::DisplayManager* dm = static_cast<vislink::DisplayManager*>(displayManager);
		dm->sync();
		return dm->frame;
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

		//return tex.id;
		return 5;
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
	if (currentDisplayManager) {
		s_RendererType = s_Graphics->GetRenderer();
		//if (s_RendererType == kUnityGfxRendererOpenGLCore) {
			currentDisplayManager->createContextTextures();
		//}
	}
}

// Freely defined function to pass a callback to plugin-specific scripts
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
GetCreateTexturesFunc()
{
	return OnCreateTextures;
}