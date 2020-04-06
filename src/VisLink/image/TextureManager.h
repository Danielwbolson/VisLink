#ifndef VISLINK_TEXTURE_TEXTURE_MANAGER_H_
#define VISLINK_TEXTURE_TEXTURE_MANAGER_H_

#include <map>
#include <string>
#include "VisLink/image/Texture.h"
#include "VisLink/sync/Semaphore.h"

namespace vislink {

class TextureManagerState;

class TextureManager {
public:
	TextureManager();
	~TextureManager();

	void createSharedTexture(const std::string& name, const TextureInfo& info, int deviceIndex);
	Texture getSharedTexture(const std::string& name, int deviceIndex);
	Semaphore getSemaphore(const std::string& name, int deviceIndex);

private:
    TextureManagerState* state;
};

}

#endif