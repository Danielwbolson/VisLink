#ifndef VISLINK_TEXTURE_TEXTURE_MANAGER_H_
#define VISLINK_TEXTURE_TEXTURE_MANAGER_H_

#include <map>
#include <string>
#include "VisLink/image/Texture.h"

namespace vislink {

class TextureManagerState;

class TextureManager {
public:
	TextureManager();
	~TextureManager();

	void createSharedTexture(const std::string& name, const TextureInfo& info);
	Texture getSharedTexture(const std::string& name);

private:
    TextureManagerState* state;
};

}

#endif