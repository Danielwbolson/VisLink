#ifndef VISLINK_SERVER_H_
#define VISLINK_SERVER_H_

#include <string>
#include <vector>

#include "VisLink/net/NetInterface.h"
#include "VisLink/impl/VisLinkAPIImpl.h"

namespace vislink {

class Server : public NetInterface {
public:
	Server(int listenPort = 3457, int numExpectedClients = 0);
	~Server();

	void createSharedTexture(const std::string& name, const TextureInfo& info, int deviceIndex) { 
		impl.createSharedTexture(name, info, deviceIndex); 
	}
	Texture getSharedTexture(const std::string& name, int deviceIndex) { return impl.getSharedTexture(name, deviceIndex); }

	void service();

private:
	SOCKET serverSocketFD;
	std::vector<SOCKET> clientSocketFDs;
	VisLinkAPIImpl impl;
	fd_set readfds;
};

}

#endif