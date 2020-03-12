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

	MessageQueue* getMessageQueue(const std::string& name) { return NULL; }

private:
	TextureManager textureManager;
};

class VisLinkOpenGL : public VisLinkAPI {
public:
	VisLinkOpenGL(VisLinkAPI* api, ProcLoader* procLoader = NULL) : api(api), deleteApi(true), procLoader(procLoader) {}
	VisLinkOpenGL(VisLinkAPI& api, ProcLoader* procLoader = NULL) : api(&api), deleteApi(false), procLoader(procLoader) {}
	~VisLinkOpenGL() {
		for (std::map<std::string, OpenGLTexture*>::iterator it = textures.begin(); it != textures.end(); it++) {
			delete it->second;
		}
		
		if (deleteApi) {
			delete api; 
		}

		if (procLoader) {
			delete procLoader;
		}
	}

	void createSharedTexture(const std::string& name, const TextureInfo& info, int deviceIndex) {
		std::cout << "createSharedTexture" << std::endl;
		api->createSharedTexture(name, info, deviceIndex);
	}

	virtual Texture getSharedTexture(const std::string& name, int deviceIndex) {
		std::cout << "getSharedTexture" << std::endl;
		Texture tex = api->getSharedTexture(name, deviceIndex);
		OpenGLTexture* openGlTexture = createOpenGLTexture(tex, procLoader);
		tex.id = openGlTexture->getId();
		textures[name] = openGlTexture;
		return tex;
	}

	MessageQueue* getMessageQueue(const std::string& name) { return api->getMessageQueue(name); }

private:
	VisLinkAPI* api;
	bool deleteApi;
	std::map<std::string, OpenGLTexture*> textures;
	ProcLoader* procLoader;
};

}


#endif