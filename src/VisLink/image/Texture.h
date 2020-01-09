#ifndef VISLINK_IMAGE_TEXTURE_H_
#define VISLINK_IMAGE_TEXTURE_H_

#include <windows.h>
#include <iostream>

namespace vislink {

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
};

class OpenGLTexture {
public:
	virtual ~OpenGLTexture() {}
	virtual unsigned int getId() const = 0;
	virtual const Texture& getTexture() const = 0;
};

OpenGLTexture* createOpenGLTexture(const Texture& texture);


}


#endif