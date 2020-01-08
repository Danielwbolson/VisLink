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
	    sendMessage(socketFD, MSG_getSharedTexture, (const unsigned char*)name.c_str(), sizeof(name.c_str()));
	    Texture tex;
	    int fd = NetInterface::recvfd(socketFD);
	    receiveData(socketFD, (unsigned char*)&tex, sizeof(tex));
	    tex.externalHandle = fd;
		return tex;
	}

private:
	SOCKET socketFD;
};

}

#endif