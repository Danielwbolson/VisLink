#ifndef VISLINK_CLIENT_H_
#define VISLINK_CLIENT_H_

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <stdint.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#else
#define SOCKET int
#include "stdint.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#ifndef WIN32
  #include <netinet/tcp.h>
  #include <netdb.h>
  #include <arpa/inet.h>
  #include <sys/wait.h>
  #include <signal.h>
	#include <sys/un.h>
#endif

#include <string>
#include <iostream>

#include "VisLink/VisLinkAPI.h"

namespace vislink {

class Client : public VisLinkAPI  {
public:
	Client(const std::string &serverIP = "127.0.0.1", int serverPort = 3457);
	~Client();

	virtual void createSharedTexture(const std::string& name, const TextureInfo& info) {}
	virtual Texture getSharedTexture(const std::string& name) { 
		static Texture tex;
		return tex;
	}

	int recvfd();
	int recvfd(int socket);

private:
	SOCKET socketFD;
};

}

#endif