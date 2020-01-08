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

	#include <stdio.h>  
	#include <string.h>   //strlen  
	#include <stdlib.h>  
	#include <errno.h>  
	#include <unistd.h>   //close  
	#include <arpa/inet.h>    //close  
	#include <sys/types.h>  
	#include <sys/socket.h>  
	#include <netinet/in.h>  
	#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#endif

#include "VisLink/VisLinkAPI.h"

namespace vislink {

class NetInterface : public VisLinkAPI {
public:
	virtual ~NetInterface() {}
	
	int sendData(SOCKET s, const unsigned char *buf, int len);
	int receiveData(SOCKET s, unsigned char *buf, int len);
	int sendfd(SOCKET socket, int fd);
	int recvfd(SOCKET socket);

};

}

#endif