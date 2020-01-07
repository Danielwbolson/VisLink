#ifndef VISLINK_NET_NET_INTERFACE_H_
#define VISLINK_NET_NET_INTERFACE_H_

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

#include "VisLink/VisLinkAPI.h"

namespace vislink {

class NetInterface : public VisLinkAPI {
public:
	virtual ~NetInterface() {}
	
	int recvfd(SOCKET socket);
	int sendfd(SOCKET socket, int fd);

};

}

#endif