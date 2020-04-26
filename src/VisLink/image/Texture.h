#ifndef VISLINK_IMAGE_TEXTURE_H_
#define VISLINK_IMAGE_TEXTURE_H_

#ifdef WIN32
#include <windows.h>
#endif
#include <iostream>

namespace vislink {

typedef void(* VLProc) (void);
class ProcLoader {
public:
	virtual ~ProcLoader() {}
	virtual VLProc getProc(const char* name) = 0;
};

enum TextureFormat {
	TEXTURE_FORMAT_RGBA8_UNORM = 0,
	TEXTURE_FORMAT_RGBA16_UNORM = 1,
	TEXTURE_FORMAT_RGBA32_UINT = 2,
	TEXTURE_FORMAT_DEPTH32F = 3
};

struct TextureInfo {
	TextureInfo() : format(TEXTURE_FORMAT_RGBA8_UNORM) {}
	int width;
	int height;
	int components;
	TextureFormat format;
};

struct Texture;

class TextureSync {
public:
	virtual void signalWrite(Texture& texture) = 0;
	virtual void waitForWrite(Texture& texture) = 0;
	virtual void signalRead(Texture& texture) = 0;
	virtual void waitForRead(Texture& texture) = 0;
};

#define NUM_TEXTURE_SEMAPHORES 2

struct Texture : TextureInfo { 
	Texture() : syncImpl(NULL) {}
#ifdef WIN32
	HANDLE externalHandle;
	HANDLE externalSemaphores[NUM_TEXTURE_SEMAPHORES];
#else
	unsigned int externalHandle;
	unsigned int externalSemaphores[NUM_TEXTURE_SEMAPHORES];
#endif
	unsigned int id;
	unsigned int semaphores[NUM_TEXTURE_SEMAPHORES];
	int deviceIndex;
	
	void signalWrite() { if(syncImpl) {syncImpl->signalWrite(*this);} }
	void waitForWrite() { if(syncImpl) {syncImpl->waitForWrite(*this);} }
	void signalRead() { if(syncImpl) {syncImpl->signalRead(*this);} }
	void waitForRead() { if(syncImpl) {syncImpl->waitForRead(*this);} }

	TextureSync* syncImpl;
};

class OpenGLTexture {
public:
	virtual ~OpenGLTexture() {}
	virtual unsigned int getId() const = 0;
	virtual const Texture& getTexture() const = 0;
};

OpenGLTexture* createOpenGLTexture(const Texture& texture, ProcLoader* procLoader = NULL);

}


#endif