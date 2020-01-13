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
#include <VisLink/display/TextureDisplay.h>

#include "sandbox/image/Image.h"

using namespace sandbox;
using namespace vislink;
using namespace std;


int main(int argc, char**argv) {
    glfwInit();

	vislink::VisLinkAPI* api = new vislink::VisLinkAPIImpl();
    api->createSharedTexture("leftWall", TextureInfo(), 0);
    api->createSharedTexture("frontWall", TextureInfo(), 0);
    api->createSharedTexture("rightWall", TextureInfo(), 1);
    api->createSharedTexture("floor", TextureInfo(), 1);

    std::vector<vislink::Display*> displays;
    displays.push_back(new TextureDisplay(api->getSharedTexture("leftWall"), 256, 256, 0, 100));
    displays.push_back(new TextureDisplay(api->getSharedTexture("frontWall"), 256, 256, 256, 100));
    displays.push_back(new TextureDisplay(api->getSharedTexture("rightWall"), 256, 256, 512, 100));
    displays.push_back(new TextureDisplay(api->getSharedTexture("floor"), 256, 256, 256, 356));

    EntityNode mainImage; 
    mainImage.addComponent(new Image("app/textures/test.png"));
    mainImage.update();
    Image* image = mainImage.getComponent<Image>();

    api = new VisLinkOpenGL(api);

    displays[0]->useContext();
    {
        GLuint format = GL_RGBA; 
        GLuint internalFormat = GL_RGBA;
        GLuint type = GL_UNSIGNED_BYTE;

        Texture tex = api->getSharedTexture("leftWall");
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->getWidth(), image->getHeight(), internalFormat, type, image->getData());
    }
    {
        GLuint format = GL_RGBA; 
        GLuint internalFormat = GL_RGBA;
        GLuint type = GL_UNSIGNED_BYTE;

        Texture tex = api->getSharedTexture("rightWall");
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->getWidth(), image->getHeight(), internalFormat, type, image->getData());
    }
    displays[0]->releaseContext();

    while(true) {
        for (int f = 0; f < displays.size(); f++) {
            displays[f]->render();
        }

        for (int f = 0; f < displays.size(); f++) {
            displays[f]->finish();
        }

        for (int f = 0; f < displays.size(); f++) {
            displays[f]->display();
        }
    }

    for (int f = 0; f < displays.size(); f++) {
        delete displays[f];
    }

    delete api;

    glfwTerminate();


	return 0;
}


