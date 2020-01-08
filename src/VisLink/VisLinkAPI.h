#ifndef VISLINK_VISLINKAPI_H_
#define VISLINK_VISLINKAPI_H_

#include <iostream>
#include "VisLink/image/Texture.h"

namespace vislink {

class VisLinkAPI {
public:
	virtual ~VisLinkAPI() {}
	virtual void createSharedTexture(const std::string& name, const TextureInfo& info) = 0;
	virtual Texture getSharedTexture(const std::string& name) = 0;
};


}


#endif