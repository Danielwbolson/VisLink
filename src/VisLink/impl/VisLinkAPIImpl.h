#ifndef VISLINK_IMPL_VISLINKAPIIMPL_H_
#define VISLINK_IMPL_VISLINKAPIIMPL_H_

#include "VisLink/VisLinkAPI.h"
#include "VisLink/image/TextureManager.h"
#include <iostream>
#include <map>

namespace vislink {

class VisLinkAPIImpl : public VisLinkAPI {
public:
	void createSharedTexture(const std::string& name, const TextureInfo& info, int deviceIndex) {
		textureManager.createSharedTexture(name, info, deviceIndex);
	}

	Texture getSharedTexture(const std::string& name, int deviceIndex) {
		return textureManager.getSharedTexture(name, deviceIndex);
	}

private:
	TextureManager textureManager;
};

class VisLinkOpenGL : public VisLinkAPI {
public:
	VisLinkOpenGL(VisLinkAPI* api) : api(api), deleteApi(true) {}
	VisLinkOpenGL(VisLinkAPI& api) : api(&api), deleteApi(false) {}
	~VisLinkOpenGL() {
		for (std::map<std::string, OpenGLTexture*>::iterator it = textures.begin(); it != textures.end(); it++) {
			delete it->second;
		}
		
		if (deleteApi) {
			delete api; 
		}
	}

	void createSharedTexture(const std::string& name, const TextureInfo& info, int deviceIndex) {
		api->createSharedTexture(name, info, deviceIndex);
	}

	virtual Texture getSharedTexture(const std::string& name, int deviceIndex) {
		Texture tex = api->getSharedTexture(name, deviceIndex);
		OpenGLTexture* openGlTexture = createOpenGLTexture(tex);
		tex.id = openGlTexture->getId();
		textures[name] = openGlTexture;
		return tex;
	}

private:
	VisLinkAPI* api;
	bool deleteApi;
	std::map<std::string, OpenGLTexture*> textures;
};

}


#endif