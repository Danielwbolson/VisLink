#ifndef VISLINK_IMPL_VISLINKAPIIMPL_H_
#define VISLINK_IMPL_VISLINKAPIIMPL_H_

#include "VisLink/VisLinkAPI.h"
#include "VisLink/image/TextureManager.h"
#include <iostream>

namespace vislink {

class VisLinkAPIImpl : public VisLinkAPI {
public:
	void createSharedTexture(const std::string& name, const TextureInfo& info) {
		textureManager.createSharedTexture(name, info);
	}

	virtual Texture getSharedTexture(const std::string& name) {
		return textureManager.getSharedTexture(name);
	}

private:
	TextureManager textureManager;
};


}


#endif