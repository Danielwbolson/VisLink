#include "VisLink/display/TextureDisplay.h"

#include "OpenGL.h"
#include <GLFW/glfw3.h>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>


namespace vislink {
	struct TextureDisplayState {
		GLFWwindow* window;
		OpenGLTexture* left;
		OpenGLTexture* right;
		GLuint vbo, vao, vshader, fshader, shaderProgram;
	};

	TextureDisplay::TextureDisplay(Texture tex, int width, int height, int xPos, int yPos, int xTexOffset, int yTexOffset) 
	 : left(tex), right(tex), xPos(xPos), yPos(yPos), width(width), height(height), xTexOffset(0), yTexOffset(0), stereo(false), frame(0) {
	 	
	 	//init();
	}

	TextureDisplay::TextureDisplay(Texture left, Texture right, int width, int height, int xPos, int yPos, int xTexOffset, int yTexOffset)
	 : left(left), right(right), xPos(xPos), yPos(yPos), width(width), height(height), xTexOffset(0), yTexOffset(0), stereo(true), frame(0) {
	 	
	 	//init();

	}

	/// Compiles shader
	GLuint TextureDisplaycompileShader(const std::string& shaderText, GLuint shaderType) {
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
	void TextureDisplaylinkShaderProgram(GLuint shaderProgram) {
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

	void TextureDisplay::init() {
		state = new TextureDisplayState();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, false);
    	glfwWindowHint(GLFW_DECORATED, false);
    	glfwWindowHint(GLFW_STEREO, stereo);

		/* GLFW says: If you wish to set an initial window position, you should create a hidden window, set its position, then show it*/
		glfwWindowHint(GLFW_VISIBLE, false);

	    state->window = glfwCreateWindow(width, height, "Window", nullptr, nullptr);
		std::cout << "Create Window" << std::endl;
	    if (!state->window) {
			std::cout << "Non Stereo" << std::endl;
    		glfwWindowHint(GLFW_STEREO, false);
    		state->window = glfwCreateWindow(width, height, "Window", nullptr, nullptr);
    		stereo = false;
	    }

	    glfwSetWindowPos (state->window, xPos, yPos);
		glfwShowWindow(state->window);

	    glfwMakeContextCurrent(state->window);
	    initializeGLExtentions();
	    glfwSwapInterval(1);

        glDisable(GL_DEPTH_TEST);
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
        GLfloat colors[]    = { 0, 0, 0,   1, 0, 0,   1, 1, 0,   1, 1, 0,   0, 1, 0 ,  0, 0, 0};    // v6-v5-v4

        // Allocate space and send Vertex Data
        glGenBuffers(1, &state->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals)+sizeof(colors), 0, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), normals);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals), sizeof(colors), colors);

        // Create vao
        glGenVertexArrays(1, &state->vao);
        glBindVertexArray(state->vao);
        glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
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
               // "uniform mat4 ProjectionMatrix; "
               // "uniform mat4 ViewMatrix; "
               // "uniform mat4 ModelMatrix; "
               // "uniform mat4 NormalMatrix; "
                ""
                "out vec3 col;"
                ""
                "void main() { "
                "   gl_Position = vec4(position, 1.0); "
                "   col = color;"
                "}";
        state->vshader = TextureDisplaycompileShader(vertexShader, GL_VERTEX_SHADER);

        std::string fragmentShader =
                "#version 330 \n"
                "in vec3 col;"
                "out vec4 colorOut;"
                "uniform sampler2D tex; "
                ""
                "void main() { "
                "   vec2 coord = vec2(col.x, col.y);"
                "   vec4 texColor = texture(tex, coord);"
                "   colorOut = texColor; "
                "}";
        state->fshader = TextureDisplaycompileShader(fragmentShader, GL_FRAGMENT_SHADER); 

        // Create shader program
        state->shaderProgram = glCreateProgram();
        glAttachShader(state->shaderProgram, state->vshader);
        glAttachShader(state->shaderProgram, state->fshader);
        TextureDisplaylinkShaderProgram(state->shaderProgram);

		glActiveTexture(GL_TEXTURE0 + 0);
		GLint loc = glGetUniformLocation(state->shaderProgram, "tex");
		glUniform1i(loc, 0);

        GLuint format = GL_RGBA;
        GLuint internalFormat = GL_RGBA;
        GLuint type = GL_UNSIGNED_BYTE;

        state->left = createOpenGLTexture(left);
        state->right = createOpenGLTexture(right);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	    glfwMakeContextCurrent(0);
	}

	TextureDisplay::~TextureDisplay() {
		glfwMakeContextCurrent(state->window);
		delete state->left;
		delete state->right;
		glfwMakeContextCurrent(0);
		glfwDestroyWindow(state->window);
		delete state;
	}

	void TextureDisplay::render() {
		glfwMakeContextCurrent(state->window);

        if (stereo) {
        	glDrawBuffer(GL_FRONT_LEFT);
        	glBindTexture(GL_TEXTURE_2D, state->left->getId());
        	renderTexture();
        	glDrawBuffer(GL_FRONT_RIGHT);
        	glBindTexture(GL_TEXTURE_2D, state->right->getId());
        	renderTexture();
        }
        else {
			//if (frame % 2 == 0) {
				glBindTexture(GL_TEXTURE_2D, state->left->getId());
			//}
			//else {
			//	glBindTexture(GL_TEXTURE_2D, state->right->getId());
			//}
        	renderTexture();
        }

		glFlush();
		glfwMakeContextCurrent(0);
	}

	void TextureDisplay::renderTexture() {
		//glClearColor(1,1,1,1);
        //glClear(GL_COLOR_BUFFER_BIT);

        /*glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);*/

        // Set shader parameters
        glUseProgram(state->shaderProgram);
		/*GLint loc = glGetUniformLocation(state->shaderProgram, "ProjectionMatrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
        loc = glGetUniformLocation(state->shaderProgram, "ViewMatrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
        loc = glGetUniformLocation(state->shaderProgram, "ModelMatrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));*/

        // Draw quad
        glBindVertexArray(state->vao);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // reset program
        glUseProgram(0);
	}

	void TextureDisplay::finish() {
		glfwMakeContextCurrent(state->window);
		glFinish();
		glfwMakeContextCurrent(0);
	}

	void TextureDisplay::display() {
		glfwMakeContextCurrent(state->window);
		glfwSwapBuffers(state->window);
		glfwMakeContextCurrent(0);
		frame++;
	}

	void TextureDisplay::useContext() {
		glfwMakeContextCurrent(state->window);
	}

	void TextureDisplay::releaseContext() {
		glfwMakeContextCurrent(0);
	}
}
