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

struct TextureInfo {
	int width;
	int height;
	int components;
};

struct Texture;

class TextureSync {
public:
	virtual void signalWrite(Texture& texture) = 0;
	virtual void waitForWrite(Texture& texture) = 0;
	virtual void signalRead(Texture& texture) = 0;
	virtual void waitForRead(Texture& texture) = 0;
};

struct Texture : TextureInfo { 
	Texture() : syncImpl(NULL) {}
#ifdef WIN32
	HANDLE externalHandle;
	HANDLE externalSemaphores[4];
#else
	unsigned int externalHandle;
	unsigned int externalSemaphores[4];
#endif
	unsigned int id;
	unsigned int semaphores[4];
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