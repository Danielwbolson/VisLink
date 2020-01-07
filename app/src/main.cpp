#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "OpenGL.h"

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

#define glCreateMemoryObjectsEXT pfnCreateMemoryObjectsEXT
PFNGLCREATEMEMORYOBJECTSEXTPROC pfnCreateMemoryObjectsEXT;
#define glImportMemoryFdEXT pfnImportMemoryFdEXT
PFNGLIMPORTMEMORYFDEXTPROC pfnImportMemoryFdEXT;
#define glTextureStorageMem2DEXT pfnTextureStorageMem2DEXT
PFNGLTEXTURESTORAGEMEM2DEXTPROC pfnTextureStorageMem2DEXT;

void initGLFW() { 
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

    window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", nullptr, nullptr);
    //glfwSetWindowUserPointer(window, this);
    glfwSetWindowPos (window, 0, 0);
    //glGetProcAddress("glGetVkProcAddrNV");
    //glGetVkProcAddrNV("glDrawVkImageNV");
    //glDrawVkImageNV((GLuint64)imageState->image, 0, 0,0, 100,100,0,0,0,100,100);

    glfwMakeContextCurrent(window);
    initializeGLExtentions();
    pfnCreateMemoryObjectsEXT = (PFNGLCREATEMEMORYOBJECTSEXTPROC)
    glfwGetProcAddress("glCreateMemoryObjectsEXT");
    pfnImportMemoryFdEXT = (PFNGLIMPORTMEMORYFDEXTPROC)
    glfwGetProcAddress("glImportMemoryFdEXT");
    pfnTextureStorageMem2DEXT = (PFNGLTEXTURESTORAGEMEM2DEXTPROC)
    glfwGetProcAddress("glTextureStorageMem2DEXT");
    
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
            mainImage.addComponent(new Image("../sandbox/examples/VulkanSandbox/textures/texture.jpg"));
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

#include <errno.h>
#include <string.h>

#include <iostream>
#include <sys/types.h>        /* See NOTES */
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>

#define LOGD(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGE(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGW(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)


using namespace std;

/**
 * Sends given file descriptior via given socket
 *
 * @param socket to be used for fd sending
 * @param fd to be sent
 * @return sendmsg result
 *
 * @note socket should be (PF_UNIX, SOCK_DGRAM)
 */
int sendfd(int socket, int fd) {
    char dummy = '$';
    struct msghdr msg;
    struct iovec iov;

    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    iov.iov_base = &dummy;
    iov.iov_len = sizeof(dummy);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = CMSG_LEN(sizeof(int));

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));

    *(int*) CMSG_DATA(cmsg) = fd;

    int ret = sendmsg(socket, &msg, 0);

    if (ret == -1) {
        LOGE("sendmsg failed with %s", strerror(errno));
    }

    return ret;
}

/**
 * Receives file descriptor using given socket
 *
 * @param socket to be used for fd recepion
 * @return received file descriptor; -1 if failed
 *
 * @note socket should be (PF_UNIX, SOCK_DGRAM)
 */
int recvfd(int socket) {
    int len;
    int fd;
    char buf[1];
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *cmsg;
    char cms[CMSG_SPACE(sizeof(int))];

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = (caddr_t) cms;
    msg.msg_controllen = sizeof cms;

    std::cout << sizeof cms  << " " << msg.msg_control << " " << msg.msg_controllen << std::endl;

    len = recvmsg(socket, &msg, 0);
    std::cout << sizeof cms  << " " << msg.msg_control << " " << msg.msg_controllen << std::endl;

    if (len < 0) {
        LOGE("recvmsg failed with %s", strerror(errno));
        return -1;
    }

    if (len == 0) {
        LOGE("recvmsg failed no data");
        return -1;
    }

    cmsg = CMSG_FIRSTHDR(&msg);
    memmove(&fd, CMSG_DATA(cmsg), sizeof(int));
    return fd;
}

#define TXTSRV "server\n"
#define TXTCLI "client\n"

void main_server(int socket) {
    int fd;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char filename[] = "/tmp/file";
    fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, mode);

    write(fd, TXTSRV, strlen(TXTSRV));
    sendfd(socket, fd);
    std::cout << "server fd: " << fd << std::endl;
    close(fd);
}

void main_client(int socket) {
    int fd = recvfd(socket);
    std::cout << "client fd: " << fd << std::endl;
    write(fd, TXTCLI, strlen(TXTCLI));
    close(fd);
}

int main2() {
    int pid;
    int pair[2];

    cout << "started..." << endl;

    if (socketpair(PF_UNIX, SOCK_DGRAM, 0, pair) < 0) {
        cout << "socketpair failed" << endl;
        return 1;
    }

    if ((pid = fork()) < 0) {
        cout << "fork failed" << endl;
        return 1;
    }

    if (pid != 0) {
        cout << "i am a parent" << endl;
        close(pair[0]);
        main_server(pair[1]);
    } else {
        cout << "i am a child" << endl;
        close(pair[1]);
        main_client(pair[0]);
    }

    return 0;
}

int main(int argc, char**argv) {


	vislink::VisLinkAPI* api = NULL;


    cout << "started..." << endl;

	int pair[2];
    if (socketpair(PF_UNIX, SOCK_DGRAM, 0, pair) < 0) {
        cout << "socketpair failed" << endl;
        return 1;
    }

	int pid;
 	if ((pid = fork()) < 0) {
        cout << "fork failed" << endl;
        return 1;
    }

    int externalHandle;
	
	if (pid == 0) {//argc > 1) {
		vislink::Client* client = new vislink::Client();
		api = client;
        close(pair[1]);
		int fd = recvfd(pair[0]);
		std::cout <<"recvfd " << fd << std::endl;
		//int externalHandle = client->recvfd();
		//std::cout << "Clientfd: " << externalHandle << std::endl;
		externalHandle = fd;
		//exit(0);
	}
	else {
		vislink::Server* server = new vislink::Server();
		api = server;
		externalHandle = api->getSharedTexture("hi")->externalHandle;

		close(pair[0]);
		sendfd(pair[1], 21);

		//server->sendfd(29);
	}

	initGLFW();
	initGL();

	//api->getSharedTexture("hi");
	//int externalHandle = api->getSharedTexture("hi")->externalHandle;
    std::cout << externalHandle << std::endl;
    GLuint mem = 0;
    glCreateMemoryObjectsEXT(1, &mem);
#ifdef WIN32
#else
    glImportMemoryFdEXT(mem, 256*256*4, GL_HANDLE_TYPE_OPAQUE_FD_EXT, externalHandle);
#endif
    glCreateTextures(GL_TEXTURE_2D, 1, &externalTexture);

    glTextureStorageMem2DEXT(externalTexture, 1, GL_RGBA8, 256, 256, mem, 0 );


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


