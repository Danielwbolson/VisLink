#ifndef VISLINK_TEXTURE_TEXTURE_MANAGER_H_
#define VISLINK_TEXTURE_TEXTURE_MANAGER_H_

#include <map>
#include <string>

namespace vislink {

class TextureManagerState;

class TextureManager {
public:
	TextureManager();
	~TextureManager();

	int externalHandle;

private:
    TextureManagerState* state;
};

}

#endif