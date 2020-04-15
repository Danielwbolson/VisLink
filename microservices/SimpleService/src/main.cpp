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

using namespace vislink;
using namespace std;

GLFWwindow* window;
GLuint vbo, vao, vshader, fshader, shaderProgram, inputTexture, outputTexture;

    /// Compiles shader
GLuint compileShader(const std::string& shaderText, GLuint shaderType) {
    const char* source = shaderText.c_str();
    int length = (int)shaderText.size();
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, &length);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetShaderInfoLog(shader, length, &length, &log[0]);
        std::cerr << &log[0];
    }

    return shader;
}

/// links shader program
void linkShaderProgram(GLuint shaderProgram) {
    glLinkProgram(shaderProgram);
    GLint status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetProgramInfoLog(shaderProgram, length, &length, &log[0]);
        std::cerr << "Error compiling program: " << &log[0] << std::endl;
    }
}

void initGL() {
    // Init GL
            glEnable(GL_DEPTH_TEST);
            glClearDepth(1.0f);
            glDepthFunc(GL_LEQUAL);
            glClearColor(0, 0, 0, 1);

            // Create VBO
            GLfloat vertices[]  = { 
                -1.0f, -1.0f, 0.0f, 
                1.0f, -1.0f, 0.0f,
                1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 0.0f,
                -1.0f, 1.0f, 0.0f, 
                -1.0f, -1.0f, 0.0f};


            // normal array
            GLfloat normals[]   = { 0, 0, 1,   0, 0, 1,   0, 0, 1,    0, 0, 1,   0, 0, 1,  0, 0, 1    };    // v6-v5-v4


            // color array
            GLfloat colors[]    = { 1, 0, 0,   0, 0, 0,   0, 1, 0,   0, 1, 0,   1, 1, 0 ,  1, 0, 0};    // v6-v5-v4

            // Allocate space and send Vertex Data
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals)+sizeof(colors), 0, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), normals);
            glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals), sizeof(colors), colors);

            // Create vao
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (char*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (char*)0 + sizeof(vertices));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (char*)0 + sizeof(vertices) + sizeof(normals));

            // Create shader
            std::string vertexShader =
                    "#version 330 \n"
                    "layout(location = 0) in vec3 position; "
                    "layout(location = 1) in vec3 normal; "
                    "layout(location = 2) in vec3 color; "
                    "out vec3 col;" 
                    ""
                    "void main() { "
                    "   gl_Position = vec4(position, 1.0); "
                    "   col = color; "
                    "}";
            vshader = compileShader(vertexShader, GL_VERTEX_SHADER);

            std::string fragmentShader =
                    "#version 330 \n"
                    "in vec3 col;"
                    "layout(location = 0) out vec4 colorOut;"
                    "uniform sampler2D tex; "
                    ""
                    "void main() { "
                    "   vec4 texColor = texture(tex, vec2(1.0-col.x,col.y));"
                    "   colorOut = vec4(texColor.x*1.0,0,0,1); "
                    "}";
            fshader = compileShader(fragmentShader, GL_FRAGMENT_SHADER); 

            // Create shader program
            shaderProgram = glCreateProgram();
            glAttachShader(shaderProgram, vshader);
            glAttachShader(shaderProgram, fshader);
            linkShaderProgram(shaderProgram);


            GLuint format = GL_RGBA;
            GLuint internalFormat = GL_RGBA;
            GLuint type = GL_UNSIGNED_BYTE;

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

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

	if (argc < 2) {
		std::cout << "Usage: ./build/bin/SimpleService <name>" << std::endl;
		exit(0);
	}

	std::string serviceName = std::string(argv[1]);

    vislink::Client* client = new vislink::Client();
    vislink::VisLinkAPI* api = client;
	TextureInfo texInfo;
	texInfo.width = 1024;
	texInfo.height = 1024;
	texInfo.components = 4;
	api->createSharedTexture(serviceName + "-input", texInfo);
	api->createSharedTexture(serviceName + "-output", texInfo);

    initGLFW();
    initGL();

    api = new vislink::VisLinkOpenGL(api);
	vislink::Texture input = api->getSharedTexture(serviceName + "-input");
	vislink::Texture output = api->getSharedTexture(serviceName + "-output");
	vislink::MessageQueue* filterStart = api->getMessageQueue(serviceName + "-start");
	vislink::MessageQueue* filterEnd = api->getMessageQueue(serviceName + "-end");

	vislink::BasicOpenGLSync beginSync;
	beginSync.addObject(filterStart);
	beginSync.addObject(api->getSemaphore(serviceName + "-ready"));
	beginSync.addObject(input);
	vislink::BasicOpenGLSync endSync;
	endSync.addObject(filterEnd);
	endSync.addObject(api->getSemaphore(serviceName + "-complete"));
	endSync.addObject(output);


    GLuint FramebufferName = 0;
    glGenFramebuffers(1, &FramebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // The depth buffer
    GLuint depthrenderbuffer;
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, output.width, output.height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, output.id, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    return false;

    // Render to our framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    glViewport(0,0,output.width, output.height); // Render on the whole framebuffer, complete from the lower left corner to the upper right

    // Set shader parameters
    glUseProgram(shaderProgram);


    while (true) {
		beginSync.waitForSignal();

        glClearColor(1,1,1,1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0+0);
        glBindTexture(GL_TEXTURE_2D, input.id);
        GLint loc = glGetUniformLocation(shaderProgram, "tex");
        glUniform1i(loc, 0);
        //std::cout << "Draw scene" << std::endl;
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

		endSync.signal();
		glFlush();

    }

    // reset program
    glUseProgram(0);

    glfwTerminate();

    delete api;

	return 0;
}


