#ifndef VISLINK_VISLINKAPI_H_
#define VISLINK_VISLINKAPI_H_

#include <iostream>
#include "VisLink/image/Texture.h"
#include "VisLink/display/TextureDisplay.h"
#include "VisLink/queue/MessageQueue.h"
#include "VisLink/sync/Semaphore.h"

namespace vislink {

class VisLinkAPI {
public:
	virtual ~VisLinkAPI() {}
	virtual void createSharedTexture(const std::string& name, const TextureInfo& info, int deviceIndex = 0) = 0;
	virtual Texture getSharedTexture(const std::string& name, int deviceIndex = 0) = 0;
	virtual MessageQueue* getMessageQueue(const std::string& name) = 0;
	virtual Semaphore getSemaphore(const std::string& name, int deviceIndex = 0) = 0;
};


}


#endif