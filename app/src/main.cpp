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

GLFWwindow* window;
GLuint vbo, vao, vshader, fshader, shaderProgram, externalTexture;
EntityNode mainImage; 
int windowXPos = 0;

void initGLFW() { 

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

    window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", nullptr, nullptr);
    glfwSetWindowPos (window, windowXPos, 20);

    glfwMakeContextCurrent(window);
    initializeGLExtentions();
    
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

    //mainImage.addComponent(new Image("app/textures/test.png"));
	mainImage.addComponent(new Image("app/textures/tex512.png"));
	//mainImage.addComponent(new Image("app/textures/tex512.jpg"));
	//mainImage.addComponent(new Image("app/textures/texture.jpg"));
	//mainImage.addComponent(new Image("app/textures/texture.png"));
    mainImage.update();
    Image* image = mainImage.getComponent<Image>();

    cout << "started..." << endl;
    glfwInit();

	vislink::VisLinkAPI* api = NULL; 

    bool server = (argc <= 1);


	if (argc > 1) {
		windowXPos = WIDTH;
	}


	int pid = 0;
    if (argc <= 1) {
#ifdef WIN32
		pid = 1;
		if (false) {
#else
        if ((pid = fork()) < 0) {
#endif
            cout << "fork failed" << endl;
            return 1;
        }

        if (pid != 0) {
            vislink::Server* server = new vislink::Server();
            api = server;
            Image* image = mainImage.getComponent<Image>();
            vislink::TextureInfo texInfo;
            texInfo.width = image->getWidth();
            texInfo.height = image->getHeight();
            texInfo.components = image->getComponents();
            api->createSharedTexture("test.png", texInfo);
            api->createSharedTexture("render", texInfo);
            while(true) {
                server->service();
            }
            return 0;
        }
    }


	if (pid == 0 || argc > 1) {
		vislink::Client* client = new vislink::Client();
		api = client;
	}
    
    vislink::MessageQueue* startFrame =  api->getMessageQueue("start");
    vislink::MessageQueue* finishFrame =  api->getMessageQueue("finish");
    vislink::MessageQueue* filterStart =  api->getMessageQueue("filterStart");
    vislink::MessageQueue* filterEnd =  api->getMessageQueue("filterEnd");

    
    initGLFW();

	//initGLFW();
	initGL();

    api = new vislink::VisLinkOpenGL(api);
    vislink::Texture tex = api->getSharedTexture("test.png");
    externalTexture = tex.id;

    if (!server &&  argc == 2) {
    	GLuint format = GL_RGBA; 
	    GLuint internalFormat = GL_RGBA;
	    GLuint type = GL_UNSIGNED_BYTE;

	    Image* image = mainImage.getComponent<Image>();

		glBindTexture(GL_TEXTURE_2D, externalTexture);
	    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->getWidth(), image->getHeight(), internalFormat, type, image->getData());

	    std::cout << "updating texture " << externalTexture << std::endl;
    }

    if (argc == 3) {
        vislink::Texture renderTex = api->getSharedTexture("render");
        externalTexture = renderTex.id;
    }


    int frame = 0;
    double lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) { 

		//std::cout << frame << std::endl;



		glfwPollEvents();


        if (frame % 100 == 0) {
            //std::cout << 
            double newTime = glfwGetTime();
            float fps = 100.0f / (newTime - lastTime);
            //std::cout << fps << std::endl;
            lastTime = newTime;
        }

        /*if (!server &&  argc == 2) {
            GLuint format = GL_RGBA; 
            GLuint internalFormat = GL_RGBA;
            GLuint type = GL_UNSIGNED_BYTE;

            Image* image = mainImage.getComponent<Image>();

            unsigned char* newImage = new unsigned char[image->getWidth()*image->getHeight()*image->getComponents()];
            std::copy(image->getData(), image->getData()+image->getWidth()*image->getHeight()*image->getComponents(), newImage);
            for (int f = 0; f < image->getWidth()*image->getHeight()*image->getComponents(); f++) {
                newImage[f] *= 1.0f*(frame%256)/256.0;
            }

            glBindTexture(GL_TEXTURE_2D, externalTexture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->getWidth(), image->getHeight(), internalFormat, type, newImage);

            delete[] newImage;
        }*/

		/*if (argc == 1) {
			startFrame->sendMessage();
			startFrame->sendObject<int>(frame);
			//std::cout << frame << std::endl;
		}
		else {
			startFrame->waitForMessage();
			//std::cout << startFrame->receiveObject<int>() << std::endl;
		}

		if (argc == 1) {
			finishFrame->waitForMessage();
		}
		else {
			finishFrame->sendMessage();
		}*/

		/*startFrame->sendMessage();
		startFrame->sendObject<int>(frame);
		finishFrame->waitForMessage();*/
        if (argc == 3) {
            filterStart->sendMessage();
            filterEnd->waitForMessage();
        }

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

        // Draw quad
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0+0);
        glBindTexture(GL_TEXTURE_2D, externalTexture);
        loc = glGetUniformLocation(shaderProgram, "tex");
        glUniform1i(loc, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // reset program
        glUseProgram(0);

		frame++;

        glfwSwapBuffers(window);


	}

    delete api;

	glfwDestroyWindow(window);
    glfwTerminate();


	return 0;
}


