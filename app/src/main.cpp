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
#include <sys/types.h>        /* See NOTES */
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>

using namespace sandbox;

GLFWwindow* window;
GLuint vbo, vao, vshader, fshader, shaderProgram, texture, externalTexture;
EntityNode mainImage;
int windowXPos = 0;

//vislink::OpenGLTexture* externalTexture = NULL;

/*#define glCreateMemoryObjectsEXT pfnCreateMemoryObjectsEXT
PFNGLCREATEMEMORYOBJECTSEXTPROC pfnCreateMemoryObjectsEXT;
#define glImportMemoryFdEXT pfnImportMemoryFdEXT
PFNGLIMPORTMEMORYFDEXTPROC pfnImportMemoryFdEXT;
#define glTextureStorageMem2DEXT pfnTextureStorageMem2DEXT
PFNGLTEXTURESTORAGEMEM2DEXTPROC pfnTextureStorageMem2DEXT;*/

void initGLFW() { 
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

    window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", nullptr, nullptr);
    //glfwSetWindowUserPointer(window, this);
    glfwSetWindowPos (window, windowXPos, 0);
    //glGetProcAddress("glGetVkProcAddrNV");
    //glGetVkProcAddrNV("glDrawVkImageNV");
    //glDrawVkImageNV((GLuint64)imageState->image, 0, 0,0, 100,100,0,0,0,100,100);

    glfwMakeContextCurrent(window);
    initializeGLExtentions();
    /*pfnCreateMemoryObjectsEXT = (PFNGLCREATEMEMORYOBJECTSEXTPROC)
    glfwGetProcAddress("glCreateMemoryObjectsEXT");
    pfnImportMemoryFdEXT = (PFNGLIMPORTMEMORYFDEXTPROC)
    glfwGetProcAddress("glImportMemoryFdEXT");
    pfnTextureStorageMem2DEXT = (PFNGLTEXTURESTORAGEMEM2DEXTPROC)
    glfwGetProcAddress("glTextureStorageMem2DEXT");*/
    
}

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
                -0.5f, -0.5f, 0.0f, 
                0.5f, -0.5f, 0.0f,
                0.5f, 0.5f, 0.0f,
                0.5f, 0.5f, 0.0f,
                -0.5f, 0.5f, 0.0f, 
                -0.5f, -0.5f, 0.0f};

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
                    ""
                    "uniform mat4 ProjectionMatrix; "
                    "uniform mat4 ViewMatrix; "
                    "uniform mat4 ModelMatrix; "
                    "uniform mat4 NormalMatrix; "
                    ""
                    "out vec3 col;"
                    ""
                    "void main() { "
                    "   gl_Position = ProjectionMatrix*ViewMatrix*ModelMatrix*vec4(position, 1.0); "
                    "   col = color;"
                    "}";
            vshader = compileShader(vertexShader, GL_VERTEX_SHADER);

            std::string fragmentShader =
                    "#version 330 \n"
                    "in vec3 col;"
                    "out vec4 colorOut;"
                    "uniform sampler2D tex; "
                    ""
                    "void main() { "
                    "   vec4 texColor = texture(tex, col.xy);"
                    "   colorOut = texColor; "
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

            glGenTextures(1, &texture);
            mainImage.addComponent(new Image("../sandbox/examples/VulkanSandbox/textures/test.png"));
            mainImage.update();
            glBindTexture(GL_TEXTURE_2D, texture);
            Image* image = mainImage.getComponent<Image>();
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image->getWidth(), image->getHeight(), 0, format, type, image->getData());

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

using namespace std;


int main(int argc, char**argv) {


	vislink::VisLinkAPI* api = NULL;


    cout << "started..." << endl;

	/*int pair[2];
    if (socketpair(PF_UNIX, SOCK_DGRAM, 0, pair) < 0) {
        cout << "socketpair failed" << endl;
        return 1;
    }

	int pid;
 	if ((pid = fork()) < 0) {
        cout << "fork failed" << endl;
        return 1;
    }*/

    int externalHandle;

    bool server = true; 

	
    if (argc > 1) {
        windowXPos = WIDTH;
    }

    initGLFW();

	//if (pid == 0) {//argc > 1) {
	if (argc > 1) {
		vislink::Client* client = new vislink::Client();
		api = client;
        //close(pair[1]);
		//int fd = recvfd(pair[0]);
		//std::cout <<"recvfd " << fd << std::endl;
		//int externalHandle = client->recvfd();
		//std::cout << "Clientfd: " << externalHandle << std::endl;
		int fd = client->recvfd();
		std::cout <<"recvfd " << fd << std::endl;
		externalHandle = fd;
		//exit(0);
		server = false;
	}
	else {
		vislink::Server* server = new vislink::Server();
        api = server;
        api->createSharedTexture("hit", vislink::TextureInfo());
        vislink::Texture tex = api->getSharedTexture("hit");
        server->sendfd(tex.externalHandle);
		/*tex = api->getSharedTexture("hit");

		server->sendfd(tex.externalHandle);

		externalHandle = tex.externalHandle;*/
	}


	initGL();


    api = new vislink::VisLinkOpenGL(api);
    vislink::Texture tex = api->getSharedTexture("hit");
    std::cout << "Texture id" << tex.id << " " << tex.externalHandle << std::endl;
    externalTexture = tex.id;

    /*tex.width = 256;
    tex.height = 256;
    tex.components = 4;
    tex.externalHandle = externalHandle;
    //vislink::OpenGLTexture* extTexture = tex.createOpenGLTexture();
    externalTexture = vislink::createOpenGLTexture(tex);*/
/*
	//api->getSharedTexture("hi");
	//int externalHandle = api->getSharedTexture("hi")->externalHandle;
    std::cout << externalHandle << std::endl;
    GLuint mem = 0;
#define SIZE 256

    glCreateMemoryObjectsEXT(1, &mem);
#ifdef WIN32
#else
    glImportMemoryFdEXT(mem, SIZE*SIZE*4, GL_HANDLE_TYPE_OPAQUE_FD_EXT, externalHandle);
#endif
    glCreateTextures(GL_TEXTURE_2D, 1, &externalTexture);

    glTextureStorageMem2DEXT(externalTexture, 1, GL_RGBA8, SIZE, SIZE, mem, 0 );
*/
    if (server) {
    	GLuint format = GL_RGBA;
	    GLuint internalFormat = GL_RGBA;
	    GLuint type = GL_UNSIGNED_BYTE;

	    Image* image = mainImage.getComponent<Image>();
	    /*glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image->getWidth(), image->getHeight(), 0, format, type, image->getData());*/

		glBindTexture(GL_TEXTURE_2D, externalTexture);
	    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->getWidth(), image->getHeight(), internalFormat, type, image->getData());

	    std::cout << "updating texture" << std::endl;
    }
    


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glfwMakeContextCurrent(window);
        glClearColor(1,1,1,1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);

        // Set shader parameters
        glUseProgram(shaderProgram);
        GLint loc = glGetUniformLocation(shaderProgram, "ProjectionMatrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
        loc = glGetUniformLocation(shaderProgram, "ViewMatrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
        loc = glGetUniformLocation(shaderProgram, "ModelMatrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

        // Draw cube
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0+0);
        //glBindTexture(GL_TEXTURE_2D, texture);
        glBindTexture(GL_TEXTURE_2D, externalTexture);
        //glBindTexture(GL_TEXTURE_2D, color[currentImage]);
        loc = glGetUniformLocation(shaderProgram, "tex");
        glUniform1i(loc, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // reset program
        glUseProgram(0);

        glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
    glfwTerminate();


	return 0;
}


