#include "VisLink/image/Texture.h"

#include "OpenGL.h"
#include <GLFW/glfw3.h>

#include <vector>

#ifdef WIN32
#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>        /* See NOTES */
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#endif

namespace vislink {

#define glCreateMemoryObjectsEXT pfnCreateMemoryObjectsEXT
PFNGLCREATEMEMORYOBJECTSEXTPROC pfnCreateMemoryObjectsEXT;

#ifdef WIN32
#define glImportMemoryWin32HandleEXT pfnImportMemoryWin32HandleEXT
PFNGLIMPORTMEMORYWIN32HANDLEEXTPROC pfnImportMemoryWin32HandleEXT;
#else
#define glImportMemoryFdEXT pfnImportMemoryFdEXT
PFNGLIMPORTMEMORYFDEXTPROC pfnImportMemoryFdEXT;
#endif

#define glTextureStorageMem2DEXT pfnTextureStorageMem2DEXT
PFNGLTEXTURESTORAGEMEM2DEXTPROC pfnTextureStorageMem2DEXT;
#define glDeleteMemoryObjectsEXT pfnDeleteMemoryObjectsEXT
PFNGLDELETEMEMORYOBJECTSEXTPROC pfnDeleteMemoryObjectsEXT;
#define glCreateTextures pfnCreateTextures
PFNGLCREATETEXTURESPROC pfnCreateTextures;



void textureInitExtensions() {
	static bool initialized = false;
	if (!initialized) {
	    pfnCreateMemoryObjectsEXT = (PFNGLCREATEMEMORYOBJECTSEXTPROC)
	    glfwGetProcAddress("glCreateMemoryObjectsEXT");
#ifdef WIN32
		pfnImportMemoryWin32HandleEXT = (PFNGLIMPORTMEMORYWIN32HANDLEEXTPROC)
			glfwGetProcAddress("glImportMemoryWin32HandleEXT");
#else
		pfnImportMemoryFdEXT = (PFNGLIMPORTMEMORYFDEXTPROC)
			glfwGetProcAddress("glImportMemoryFdEXT");
#endif
		pfnTextureStorageMem2DEXT = (PFNGLTEXTURESTORAGEMEM2DEXTPROC)
	    glfwGetProcAddress("glTextureStorageMem2DEXT");
		pfnDeleteMemoryObjectsEXT = (PFNGLDELETEMEMORYOBJECTSEXTPROC)
		glfwGetProcAddress("glDeleteMemoryObjectsEXT");
		pfnCreateTextures = (PFNGLCREATETEXTURESPROC)
		glfwGetProcAddress("glCreateTextures");
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
	Texture texture;
};

OpenGLTexture* createOpenGLTexture(const Texture& tex) {
	textureInitExtensions();


#ifdef WIN32
	HANDLE newHandle = tex.externalHandle;
#else
	int newHandle = tex.externalHandle;
	newHandle = dup(newHandle);
#endif

    std::cout << tex.externalHandle << std::endl;
    GLuint mem = 0;
    GLuint externalTexture = 0;
    glCreateMemoryObjectsEXT(1, &mem);
#ifdef WIN32
	glImportMemoryWin32HandleEXT(mem, tex.width * tex.height * tex.components, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, newHandle);
#else
    glImportMemoryFdEXT(mem, tex.width*tex.height*tex.components, GL_HANDLE_TYPE_OPAQUE_FD_EXT, newHandle);
#endif
    glCreateTextures(GL_TEXTURE_2D, 1, &externalTexture);

    glTextureStorageMem2DEXT(externalTexture, 1, GL_RGBA8, tex.width, tex.height, mem, 0 );
    OpenGLTextureImpl* texture = new OpenGLTextureImpl(tex);
    texture->mem = mem;
    texture->id = externalTexture;
	return texture;
}

}
