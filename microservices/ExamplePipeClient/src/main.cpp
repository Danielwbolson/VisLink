//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
//#include "OpenGL.h"

#include "OpenGL.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <VisLink/net/Server.h>
#include <VisLink/net/Client.h>
#include <VisLink/sync/SyncStrategy.h>

#define WIDTH 512
#define HEIGHT 512

#include "sandbox/image/Image.h"
#include "src/MicroService.h"

using namespace sandbox;
using namespace std;

void initGLFW();
void initGL();
void initFramebuffer(int width, int height, GLuint textureId);
void renderScene();

GLFWwindow* window;
GLuint vbo, vao, vshader, fshader, shaderProgram, tex;
GLuint vshaderOutput, fshaderOutput, shaderProgramOutput;
GLuint FramebufferName;
int windowXPos = 0;
bool useFilter = true;




int main(int argc, char**argv) {
	if (argc < 6) {
		std::cout << "Usage: ./build/bin/SimpleClient <service1> <service2> <inputTexture> <midTexture> <outputTexture>" << std::endl;
		exit(0);
	}

	std::string serviceName = std::string(argv[1]);
	std::string service2Name = std::string(argv[2]);
	std::string inputTextureName = std::string(argv[3]);
	std::string midTextureName = std::string(argv[4]);
	std::string outputTextureName = std::string(argv[5]);

	glfwInit();

	vislink::VisLinkAPI* api = NULL;

	initGLFW();
	initGL();

	// Load scene texture
	EntityNode mainImage;
	mainImage.addComponent(new Image("microservices/SimpleClient/textures/tex512.png"));
	mainImage.update();
	Image* image = mainImage.getComponent<Image>();
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->getWidth(), image->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image->getData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	// VisLink Connection
	vislink::Client* client = new vislink::Client();
	api = new vislink::VisLinkOpenGL(client);

	MicroService colorFilter(serviceName, api);
	vislink::Texture input = colorFilter.addTexture(inputTextureName, MICROSERVICE_INPUT_TEXTURE);
	vislink::Texture mid = colorFilter.addTexture(midTextureName, MICROSERVICE_OUTPUT_TEXTURE);

	MicroService thresholdFilter(service2Name, api);
	vislink::Texture mid2 = thresholdFilter.addTexture(midTextureName, MICROSERVICE_INPUT_TEXTURE);
	vislink::Texture output = thresholdFilter.addTexture(outputTextureName, MICROSERVICE_OUTPUT_TEXTURE);

	initFramebuffer(input.width, input.height, input.id);

	int frame = 0;
	double lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		// Render Scene
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glViewport(0, 0, input.width, input.height);
		renderScene();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Apply Filter by calling microservice
		GLuint outputTexture = input.id;
		if (useFilter) {
			vislink::MessageQueue* queue = colorFilter.begin();
			queue->sendObject(glm::vec4(0, 1, 0, 1));
			colorFilter.end();
			//outputTexture = output.id;

			queue = thresholdFilter.begin();
			queue->sendObject(glm::vec4(0, 1, 0, 1));
			thresholdFilter.end();
			outputTexture = output.id;
		}

		// Draw final result
		glViewport(0, 0, 512, 512);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glUseProgram(shaderProgramOutput);
		// Draw quad
		glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, outputTexture);
		GLint loc = glGetUniformLocation(shaderProgramOutput, "tex");
		glUniform1i(loc, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// Output frame reate information
		if (frame % 100 == 0) {
			//std::cout << 
			double newTime = glfwGetTime();
			float fps = 100.0f / (newTime - lastTime);
			std::cout << fps << std::endl;
			lastTime = newTime;
		}
		frame++;

		glfwSwapBuffers(window);
	}

	delete api;

	glfwDestroyWindow(window);
	glfwTerminate();


	return 0;
}

void renderScene() {
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), -3.14159f / 4.0f, glm::vec3(1.0, 0.0, 0.0))
		*glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.0, 0.0, 1.0))
		*glm::scale(glm::mat4(1.0f), glm::vec3(0.5));

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
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, tex);
	loc = glGetUniformLocation(shaderProgram, "tex");
	glUniform1i(loc, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	// reset program
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	if (status == GL_FALSE) {
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
	if (status == GL_FALSE) {
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
	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f };


	// normal array
	GLfloat normals[] = { 0, 0, 1,   0, 0, 1,   0, 0, 1,    0, 0, 1,   0, 0, 1,  0, 0, 1 };    // v6-v5-v4


	// color array
	GLfloat colors[] = { 1, 0, 0,   0, 0, 0,   0, 1, 0,   0, 1, 0,   1, 1, 0 ,  1, 0, 0 };    // v6-v5-v4

	// Allocate space and send Vertex Data
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(normals) + sizeof(colors), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(normals), sizeof(colors), colors);

	// Create vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (char*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (char*)0 + sizeof(vertices));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (char*)0 + sizeof(vertices) + sizeof(normals));

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
		//"   colorOut = vec4(1,0,0,1); "
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

	// Create shader
	std::string vertexShaderOutput =
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
	vshaderOutput = compileShader(vertexShaderOutput, GL_VERTEX_SHADER);

	std::string fragmentShaderOutput =
		"#version 330 \n"
		"in vec3 col;"
		"layout(location = 0) out vec4 colorOut;"
		"uniform sampler2D tex; "
		""
		"void main() { "
		"   vec4 texColor = texture(tex, col.xy);"
		"   colorOut = texColor; "
		"}";
	fshaderOutput = compileShader(fragmentShaderOutput, GL_FRAGMENT_SHADER);

	// Create shader program
	shaderProgramOutput = glCreateProgram();
	glAttachShader(shaderProgramOutput, vshaderOutput);
	glAttachShader(shaderProgramOutput, fshaderOutput);
	linkShaderProgram(shaderProgramOutput);


}

void initFramebuffer(int width, int height, GLuint textureId) {
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// The depth buffer
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureId, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_SPACE:
			useFilter = !useFilter;
			break;
		}
	}
}

void initGLFW() {
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

	window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", nullptr, nullptr);
	glfwSetWindowPos(window, windowXPos, 20);
	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);
	initializeGLExtentions();
}
