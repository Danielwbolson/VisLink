#ifndef VISLINK_IMPL_VISLINKAPIIMPL_H_
#define VISLINK_IMPL_VISLINKAPIIMPL_H_

#include "VisLink/VisLinkAPI.h"
#include "VisLink/image/TextureManager.h"
#include <iostream>
#include <map>

namespace vislink {

class VisLinkAPIImpl : public VisLinkAPI {
public:
	void createSharedTexture(const std::string& name, const TextureInfo& info) {
		textureManager.createSharedTexture(name, info);
	}

	Texture getSharedTexture(const std::string& name) {
		return textureManager.getSharedTexture(name);
	}

private:
	TextureManager textureManager;
};

class VisLinkOpenGL : public VisLinkAPI {
public:
	VisLinkOpenGL(VisLinkAPI* api) : api(api) {}
	~VisLinkOpenGL() { delete api; }

	void createSharedTexture(const std::string& name, const TextureInfo& info) {
		api->createSharedTexture(name, info);
	}

	virtual Texture getSharedTexture(const std::string& name) {
		Texture tex = api->getSharedTexture(name);
		OpenGLTexture* openGlTexture = createOpenGLTexture(tex);
		tex.id = openGlTexture->getId();
		textures[name] = openGlTexture;
		return tex;
	}

private:
	VisLinkAPI* api;
	std::map<std::string, OpenGLTexture*> textures;
};

}


#endif