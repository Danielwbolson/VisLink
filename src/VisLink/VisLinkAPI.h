#ifndef VISLINK_VISLINKAPI_H_
#define VISLINK_VISLINKAPI_H_

#include "VisLink/image/TextureManager.h"
#include <iostream>

namespace vislink {

struct TextureInfo {};
class Texture { public: unsigned int externalHandle; };

class VisLinkAPI {
public:
	virtual void createSharedTexture(const std::string& name, const TextureInfo& info) = 0;
	virtual Texture* getSharedTexture(const std::string& name) = 0;
};

class VisLinkAPIImpl : public VisLinkAPI {
public:
	void createSharedTexture(const std::string& name, const TextureInfo& info) {

	}

	virtual Texture* getSharedTexture(const std::string& name) {
		std::cout << name << " " << textureManager.externalHandle << std::endl;
		static Texture tex;
		tex.externalHandle = textureManager.externalHandle;
		return &tex;
	}

private:
	TextureManager textureManager;
};


}


#endif