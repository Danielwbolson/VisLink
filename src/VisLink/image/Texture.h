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

struct Texture : TextureInfo { 
	Texture() {}
#ifdef WIN32
	HANDLE externalHandle;
#else
	unsigned int externalHandle;
#endif
	unsigned int id;
	int deviceIndex;
	unsigned int visLinkId;
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