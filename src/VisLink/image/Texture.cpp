#include "VisLink/image/Texture.h"

#include "OpenGL.h"
#include <GLFW/glfw3.h>

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>        /* See NOTES */
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>

namespace vislink {

#define glCreateMemoryObjectsEXT pfnCreateMemoryObjectsEXT
PFNGLCREATEMEMORYOBJECTSEXTPROC pfnCreateMemoryObjectsEXT;
#define glImportMemoryFdEXT pfnImportMemoryFdEXT
PFNGLIMPORTMEMORYFDEXTPROC pfnImportMemoryFdEXT;
#define glTextureStorageMem2DEXT pfnTextureStorageMem2DEXT
PFNGLTEXTURESTORAGEMEM2DEXTPROC pfnTextureStorageMem2DEXT;
#define glDeleteMemoryObjectsEXT pfnDeleteMemoryObjectsEXT
PFNGLDELETEMEMORYOBJECTSEXTPROC pfnDeleteMemoryObjectsEXT;

void textureInitExtensions() {
	static bool initialized = false;
	if (!initialized) {
	    pfnCreateMemoryObjectsEXT = (PFNGLCREATEMEMORYOBJECTSEXTPROC)
	    glfwGetProcAddress("glCreateMemoryObjectsEXT");
	    pfnImportMemoryFdEXT = (PFNGLIMPORTMEMORYFDEXTPROC)
	    glfwGetProcAddress("glImportMemoryFdEXT");
	    pfnTextureStorageMem2DEXT = (PFNGLTEXTURESTORAGEMEM2DEXTPROC)
	    glfwGetProcAddress("glTextureStorageMem2DEXT");
	    pfnDeleteMemoryObjectsEXT = (PFNGLDELETEMEMORYOBJECTSEXTPROC)
	    glfwGetProcAddress("glDeleteMemoryObjectsEXT");
	    initialized = true;
	}
}

class OpenGLTextureImpl : public OpenGLTexture {
public:
	OpenGLTextureImpl(const Texture& texture) : texture(texture) {}
	~OpenGLTextureImpl() {
		glDeleteTextures(1, &id);
		glDeleteMemoryObjectsEXT(1, &mem);
	}
	virtual unsigned int getId() const { return id; }
	virtual const Texture& getTexture() const { return texture; }
	GLuint id;
	GLuint mem;
private:
	Texture texture;
};


OpenGLTexture* Texture::createOpenGLTexture() {
	textureInitExtensions();

	int newHandle = dup(externalHandle);

    std::cout << externalHandle << std::endl;
    GLuint mem = 0;
    GLuint externalTexture = 0;
    glCreateMemoryObjectsEXT(1, &mem);
#ifdef WIN32
#else
    glImportMemoryFdEXT(mem, width*height*components, GL_HANDLE_TYPE_OPAQUE_FD_EXT, newHandle);
#endif
    glCreateTextures(GL_TEXTURE_2D, 1, &externalTexture);

    glTextureStorageMem2DEXT(externalTexture, 1, GL_RGBA8, width, height, mem, 0 );
    OpenGLTextureImpl* texture = new OpenGLTextureImpl(*this);
    texture->mem = mem;
    texture->id = externalTexture;
	return texture;
}

}