#ifndef MICROSERVICE_H_
#define MICROSERVICE_H_

#include "OpenGL.h"
#include <GLFW/glfw3.h>

#include <string>
#include <VisLink/VisLinkAPI.h>
#include <VisLink/sync/SyncStrategy.h>
#include <vector>
#include <sstream>

enum MicroServiceTextureType {
	MICROSERVICE_INPUT_TEXTURE,
	MICROSERVICE_OUTPUT_TEXTURE
};

class MicroService {
public:
	MicroService(const std::string& name, vislink::VisLinkAPI* api) : serviceName(name), api(api) {
		beginQueue = api->getMessageQueue(name + "-begin");
		endQueue = api->getMessageQueue(name + "-end");

		beginSync.addObject(beginQueue);
		beginSync.addObject(api->getSemaphore(name + "-ready"));

		endSync.addObject(endQueue);
		endSync.addObject(api->getSemaphore(name + "-complete"));
	}
	virtual ~MicroService() {}

	const std::string& getName() { return serviceName; }

	virtual vislink::Texture addTexture(const std::string& name, MicroServiceTextureType type) {
		vislink::Texture tex = api->getSharedTexture(name);
		if (type == MICROSERVICE_INPUT_TEXTURE) {
			beginSync.addObject(vislink::ReadTexture(tex));
		}
		else {
			beginSync.addObject(vislink::WriteTexture(tex));
			endSync.addObject(vislink::ReadTexture(tex));
		}
		return tex;
	}

	vislink::MessageQueue* begin() {
		beginImpl();
		return beginQueue;
	}

	vislink::MessageQueue* end() {
		endImpl();
		return endQueue;
	}

protected:
	virtual void beginImpl() {
		beginSync.signal();
	}

	virtual void endImpl() {
		endSync.waitForSignal();
	}

	vislink::BasicOpenGLSync beginSync;
	vislink::BasicOpenGLSync endSync;
	vislink::MessageQueue* beginQueue;
	vislink::MessageQueue* endQueue;
	vislink::VisLinkAPI* api;

private:
	std::string serviceName;
};

//---------------------------------------------------------------------------------

class MicroServiceImpl : public MicroService {
public:
	MicroServiceImpl(const std::string& name, vislink::VisLinkAPI* api) : MicroService(name, api) {}

	vislink::Texture addTexture(const std::string& name, MicroServiceTextureType type) {
		vislink::TextureInfo texInfo;
		texInfo.width = 1024;
		texInfo.height = 1024;
		texInfo.components = 4;
		api->createSharedTexture(name, texInfo);
		return MicroService::addTexture(name, type);
	}

protected:
	virtual void beginImpl() {
		beginSync.waitForSignal();
	}

	virtual void endImpl() {
		endSync.signal();
	}
};

//---------------------------------------------------------------------------------------


class FrameBufferService : public MicroServiceImpl {
public:
	FrameBufferService(const std::string& name, vislink::VisLinkAPI* api) : MicroServiceImpl(name, api), initialized(false) {
	}

	vislink::Texture addTexture(const std::string& name, MicroServiceTextureType type) {
		vislink::Texture tex = MicroServiceImpl::addTexture(name, type);

		if (type == MICROSERVICE_INPUT_TEXTURE) {
			inputTextures.push_back(tex);
		}
		else if (type == MICROSERVICE_OUTPUT_TEXTURE) {
			outputTextures.push_back(tex);
		}

		return tex;
	}

protected:
	virtual void beginImpl() {
		initGL();

		MicroServiceImpl::beginImpl();

		glBindRenderbuffer(GL_RENDERBUFFER, framebufferName);

		// Set shader parameters
		glUseProgram(shaderProgram);

		setParameters(beginQueue, shaderProgram);

		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glBindVertexArray(vao);

		//std::cout << "Draw scene" << std::endl;
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// reset program
		glUseProgram(0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	virtual void endImpl() {
		MicroServiceImpl::endImpl();
		glFlush();
	}

	virtual std::string getVertexShader() = 0;
	virtual std::string getFragmentShader() = 0;
	virtual void setParameters(vislink::MessageQueue* queue, GLuint shaderProgram) = 0;

private:
	std::vector<vislink::Texture> inputTextures;
	std::vector<vislink::Texture> outputTextures;

	void initGL() {
		if (initialized) { return; }

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
		std::string vertexShader = getVertexShader();
		vshader = compileShader(vertexShader, GL_VERTEX_SHADER);

		std::string fragmentShader = getFragmentShader();
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

		vislink::Texture output = outputTextures[0];

		glGenFramebuffers(1, &framebufferName);
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);

		// Poor filtering. Needed !
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// The depth buffer
		GLuint framebufferName;
		glGenRenderbuffers(1, &framebufferName);
		glBindRenderbuffer(GL_RENDERBUFFER, framebufferName);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, output.width, output.height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferName);

		std::vector<GLuint> drawBuffers;
		// Set the list of draw buffers.
		for (int f = 0; f < outputTextures.size(); f++) {
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+f, outputTextures[f].id, 0);
			drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + f);
		}

		glDrawBuffers(outputTextures.size(), &drawBuffers[0]);

		// Always check that our framebuffer is ok
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return;

		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
		glViewport(0, 0, output.width, output.height); // Render on the whole framebuffer, complete from the lower left corner to the upper right

		glUseProgram(shaderProgram);
		for (int f = 0; f < inputTextures.size(); f++) {
			glActiveTexture(GL_TEXTURE0 + f);
			glBindTexture(GL_TEXTURE_2D, inputTextures[f].id);
			std::stringstream ss;
			ss << "inputTexture[" << f << "]";
			GLint loc = glGetUniformLocation(shaderProgram, ss.str().c_str());
			glUniform1i(loc, f);
		}
		glUseProgram(0);

		initialized = true;
	}

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

	GLuint vbo, vao, vshader, fshader, shaderProgram;
	GLuint framebufferName;
	bool initialized;
};

#endif