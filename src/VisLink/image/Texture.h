#ifndef VISLINK_IMAGE_TEXTURE_H_
#define VISLINK_IMAGE_TEXTURE_H_

#include <iostream>

namespace vislink {

struct TextureInfo {
	int width;
	int height;
	int components;
};

class OpenGLTexture;

struct Texture : TextureInfo { 
	unsigned int externalHandle;
	OpenGLTexture* createOpenGLTexture();
};

class OpenGLTexture {
public:
	virtual ~OpenGLTexture() {}
	virtual unsigned int getId() const = 0;
	virtual const Texture& getTexture() const = 0;
};


}


#endif