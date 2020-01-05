#ifndef VISLINK_VISLINKAPI_H_
#define VISLINK_VISLINKAPI_H_

namespace vislink {

struct TextureInfo {};
class Texture {};

class VisLinkAPI {
public:
	virtual void createSharedTexture(const std::string, const TextureInfo& info) = 0;
	virtual Texture* getSharedTexture(const std::string) = 0;
};


}


#endif