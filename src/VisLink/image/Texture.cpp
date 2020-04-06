#include "VisLink/image/Texture.h"
#include "VisLink/sync/Semaphore.h"

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
#define glImportSemaphoreWin32HandleEXT pfnImportSemaphoreWin32HandleEXT
PFNGLIMPORTSEMAPHOREWIN32HANDLEEXTPROC pfnImportSemaphoreWin32HandleEXT;
#else
#define glImportMemoryFdEXT pfnImportMemoryFdEXT
PFNGLIMPORTMEMORYFDEXTPROC pfnImportMemoryFdEXT;
#define glImportSemaphoreFdEXT pfnImportSemaphoreFdEXT
PFNGLIMPORTSEMAPHOREFDEXTPROC pfnImportSemaphoreFdEXT;
#endif

#define glTextureStorageMem2DEXT pfnTextureStorageMem2DEXT
PFNGLTEXTURESTORAGEMEM2DEXTPROC pfnTextureStorageMem2DEXT;
#define glDeleteMemoryObjectsEXT pfnDeleteMemoryObjectsEXT
PFNGLDELETEMEMORYOBJECTSEXTPROC pfnDeleteMemoryObjectsEXT;
#define glCreateTextures pfnCreateTextures
PFNGLCREATETEXTURESPROC pfnCreateTextures;
#define glGenSemaphoresEXT pfnGenSemaphoresEXT
PFNGLGENSEMAPHORESEXTPROC pfnGenSemaphoresEXT;
#define glSignalSemaphoreEXT pfnSignalSemaphoreEXT
PFNGLSIGNALSEMAPHOREEXTPROC pfnSignalSemaphoreEXT;
#define glWaitSemaphoreEXT pfnWaitSemaphoreEXT
PFNGLWAITSEMAPHOREEXTPROC pfnWaitSemaphoreEXT;

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

static HMODULE libgl;

static void open_libgl(void)
{
	libgl = LoadLibraryA("opengl32.dll");
}

static void close_libgl(void)
{
	FreeLibrary(libgl);
}

static void* get_proc(const char* proc)
{
	void* res;

	res = wglGetProcAddress(proc);
	if (!res)
		res = GetProcAddress(libgl, proc);
	return res;
}

void textureInitExtensions() {
	static bool initialized = false;
	if (!initialized) {
		open_libgl();
	    pfnCreateMemoryObjectsEXT = (PFNGLCREATEMEMORYOBJECTSEXTPROC)
			get_proc("glCreateMemoryObjectsEXT");
		pfnImportMemoryWin32HandleEXT = (PFNGLIMPORTMEMORYWIN32HANDLEEXTPROC)
			get_proc("glImportMemoryWin32HandleEXT");
		pfnTextureStorageMem2DEXT = (PFNGLTEXTURESTORAGEMEM2DEXTPROC)
			get_proc("glTextureStorageMem2DEXT");
		pfnDeleteMemoryObjectsEXT = (PFNGLDELETEMEMORYOBJECTSEXTPROC)
			get_proc("glDeleteMemoryObjectsEXT");
		pfnCreateTextures = (PFNGLCREATETEXTURESPROC)
			get_proc("glCreateTextures");
		pfnGenSemaphoresEXT = (PFNGLGENSEMAPHORESEXTPROC)
			get_proc("glGenSemaphoresEXT");
		pfnImportSemaphoreWin32HandleEXT = (PFNGLIMPORTSEMAPHOREWIN32HANDLEEXTPROC)
            get_proc("glImportSemaphoreWin32HandleEXT");
	    initialized = true;
	    close_libgl();
		std::cout << "wgl get proc" << std::endl;
	}
}

#else

class VLGlfwProcLoader : public ProcLoader {
public:
	VLProc getProc(const char* name) {
		return glfwGetProcAddress(name);
	}
};

void textureInitExtensions(ProcLoader* procLoader) {
	static VLGlfwProcLoader defaultLoader;
	if (!procLoader) {
		procLoader = &defaultLoader;
	}

	static bool initialized = false;
	if (!initialized) {
	    pfnCreateMemoryObjectsEXT = (PFNGLCREATEMEMORYOBJECTSEXTPROC)
			procLoader->getProc("glCreateMemoryObjectsEXT");
		pfnImportMemoryFdEXT = (PFNGLIMPORTMEMORYFDEXTPROC)
			procLoader->getProc("glImportMemoryFdEXT");
		pfnTextureStorageMem2DEXT = (PFNGLTEXTURESTORAGEMEM2DEXTPROC)
			procLoader->getProc("glTextureStorageMem2DEXT");
		pfnDeleteMemoryObjectsEXT = (PFNGLDELETEMEMORYOBJECTSEXTPROC)
			procLoader->getProc("glDeleteMemoryObjectsEXT");
		pfnCreateTextures = (PFNGLCREATETEXTURESPROC)
			procLoader->getProc("glCreateTextures");
		pfnGenSemaphoresEXT = (PFNGLGENSEMAPHORESEXTPROC)
			procLoader->getProc("glGenSemaphoresEXT");
		pfnImportSemaphoreFdEXT = (PFNGLIMPORTSEMAPHOREFDEXTPROC)
			procLoader->getProc("glImportSemaphoreFdEXT");
		pfnSignalSemaphoreEXT = (PFNGLSIGNALSEMAPHOREEXTPROC)
			procLoader->getProc("glSignalSemaphoreEXT");
		pfnWaitSemaphoreEXT = (PFNGLWAITSEMAPHOREEXTPROC)
			procLoader->getProc("glWaitSemaphoreEXT");

	    initialized = true;
	}
}

#endif


class OpenGLTextureImpl : public OpenGLTexture {
public:
	OpenGLTextureImpl(const Texture& texture) : texture(texture) {}
	~OpenGLTextureImpl() {
		//glDeleteTextures(1, &id);
		//glDeleteMemoryObjectsEXT(1, &mem);
	}
	virtual unsigned int getId() const { return id; }
	virtual const Texture& getTexture() const { return texture; }
	GLuint id;
	GLuint mem;
	Texture texture;
};

class OpenGLTextureSync : public TextureSync {
public:
	static TextureSync* getInstance() {
		static OpenGLTextureSync sync;
		return &sync;
	}

	void signalWrite(Texture& texture) { 
		GLenum dstLayout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
        glSignalSemaphoreEXT(texture.semaphores[0], 0, nullptr, 1, &texture.id, &dstLayout);
		//std::cout << "signalWrite2" << std::endl; 
	}
	void waitForWrite(Texture& texture) {
		GLenum srcLayout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
        glWaitSemaphoreEXT(texture.semaphores[0], 0, nullptr, 1, &texture.id, &srcLayout);
	}
	void signalRead(Texture& texture) {
		GLenum dstLayout = GL_LAYOUT_SHADER_READ_ONLY_EXT;
        glSignalSemaphoreEXT(texture.semaphores[1], 0, nullptr, 1, &texture.id, &dstLayout);
	}
	void waitForRead(Texture& texture) {
		GLenum srcLayout = GL_LAYOUT_SHADER_READ_ONLY_EXT;
        glWaitSemaphoreEXT(texture.semaphores[1], 0, nullptr, 1, &texture.id, &srcLayout);
	}
};

OpenGLTexture* createOpenGLTexture(const Texture& tex, ProcLoader* procLoader) {

	
#ifdef WIN32
	textureInitExtensions();
	HANDLE newHandle = tex.externalHandle;
#else
	textureInitExtensions(procLoader);
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

    glTextureStorageMem2DEXT(externalTexture, 1, GL_RGBA16F, tex.width, tex.height, mem, 0 );
    OpenGLTextureImpl* texture = new OpenGLTextureImpl(tex);
    texture->mem = mem;
    texture->id = externalTexture;
    texture->texture.id = externalTexture;
    if (false) {
    	texture->texture.syncImpl = OpenGLTextureSync::getInstance();
    }

#ifdef WIN32
    HANDLE semaphoreFDs[NUM_TEXTURE_SEMAPHORES];
	for (int f = 0; f < NUM_TEXTURE_SEMAPHORES; f++) {
		semaphoreFDs[f] = tex.externalSemaphores[f];
	}
#else
    int semaphoreFDs[NUM_TEXTURE_SEMAPHORES];
	for (int f = 0; f < NUM_TEXTURE_SEMAPHORES; f++) {
		semaphoreFDs[f] = dup(tex.externalSemaphores[f]);
	}
#endif

	glGenSemaphoresEXT(NUM_TEXTURE_SEMAPHORES, texture->texture.semaphores);

	for (int f = 0; f < NUM_TEXTURE_SEMAPHORES; f++) {
#ifdef WIN32
		glImportSemaphoreWin32HandleEXT(texture->texture.semaphores[f], GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, semaphoreFDs[f]);
#else
 		glImportSemaphoreFdEXT(texture->texture.semaphores[f], GL_HANDLE_TYPE_OPAQUE_FD_EXT, semaphoreFDs[f]);
#endif
	}


	return texture;
}

class OpenGLSemaphoreImpl : public OpenGLSemaphore {
public:
	OpenGLSemaphoreImpl(const Semaphore& semaphore) : semaphore(semaphore) {}
	~OpenGLSemaphoreImpl() {
		//glDeleteTextures(1, &id);
		//glDeleteMemoryObjectsEXT(1, &mem);
	}
	virtual unsigned int getId() const { return id; }
	virtual const Semaphore& getSemaphore() const { return semaphore; }
	GLuint id;
	Semaphore semaphore;
};


OpenGLSemaphore* createOpenGLSemaphore(const Semaphore& semaphore, ProcLoader* procLoader) {
#ifdef WIN32
	textureInitExtensions();
	HANDLE newHandle = semaphore.externalHandle;
#else
	textureInitExtensions(procLoader);
	int newHandle = semaphore.externalHandle;
	newHandle = dup(newHandle);
#endif

	OpenGLSemaphoreImpl* sem = new OpenGLSemaphoreImpl(semaphore);

    GLuint externalSemaphore = 0;
	glGenSemaphoresEXT(1, &externalSemaphore);
#ifdef WIN32
	glImportSemaphoreWin32HandleEXT(externalSemaphore, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, newHandle);
#else
 	glImportSemaphoreFdEXT(externalSemaphore, GL_HANDLE_TYPE_OPAQUE_FD_EXT, newHandle);
#endif

 	sem->id = externalSemaphore;
 	sem->semaphore.id = externalSemaphore;
	return sem;
}

void OpenGLSync::signal(const Semaphore& semaphore) {
	glSignalSemaphoreEXT(semaphore.id, 0, nullptr, textures.size(), &textures[0], &layouts[0]);
	glFlush();
}


void OpenGLSync::waitForSignal(const Semaphore& semaphore) {
	glWaitSemaphoreEXT(semaphore.id, 0, nullptr, textures.size(), &textures[0], &layouts[0]);
	glFlush();
}

void OpenGLSync::addTexture(unsigned int id, unsigned int layout) {
	textures.push_back(id);
	layouts.push_back(layout);
}

void OpenGLSync::write(const Texture& tex) {
	addTexture(tex.id, GL_LAYOUT_COLOR_ATTACHMENT_EXT);
}

void OpenGLSync::read(const Texture& tex) {
	addTexture(tex.id, GL_LAYOUT_SHADER_READ_ONLY_EXT);
}

}
