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

struct Texture : TextureInfo { 
#ifdef WIN32
	HANDLE externalHandle;
#else
	unsigned int externalHandle;
#endif
	unsigned int id;
	int deviceIndex;
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