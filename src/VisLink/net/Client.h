#ifndef VISLINK_CLIENT_H_
#define VISLINK_CLIENT_H_

#include <string>
#include <iostream>

#include "VisLink/net/NetInterface.h"
#include "VisLink/VisLinkAPI.h"

namespace vislink {

class Client : public NetInterface  {
public:
	Client(const std::string &serverIP = "127.0.0.1", int serverPort = 3457);
	~Client();

	virtual void createSharedTexture(const std::string& name, const TextureInfo& info) {}
	virtual Texture getSharedTexture(const std::string& name) { 
		static Texture tex;
		return tex;
	}

	int recvfd();

private:
	SOCKET socketFD;
};

}

#endif