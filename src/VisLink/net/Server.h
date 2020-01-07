#ifndef VISLINK_SERVER_H_
#define VISLINK_SERVER_H_

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
#include <vector>

#include "VisLink/VisLinkAPI.h"

namespace vislink {

class Server : public VisLinkAPIImpl {
public:
	Server(int listenPort = 3457, int numExpectedClients = 1);
	~Server();

	//virtual void createSharedTexture(const std::string, const TextureInfo& info) {}
	//virtual Texture* getSharedTexture(const std::string) {return NULL;}

	int sendfd(int fd);
	int sendfd(SOCKET socket, int fd);

private:
	std::vector<SOCKET> clientSocketFDs;
};

}

#endif