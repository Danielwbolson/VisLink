#ifndef VISLINK_NET_NET_INTERFACE_H_
#define VISLINK_NET_NET_INTERFACE_H_

#ifdef WIN32
#include <stdint.h>
#include <stdio.h>
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

#ifndef WIN32_LEAN_AND_MEAN
#ifndef _WINSOCK2API_
#ifndef _WINSOCKAPI_

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#endif
#endif
#endif

#include "VisLink/VisLinkAPI.h"
#include <vector>

namespace vislink {

//#define NET_SIZEOFINT 4

enum NetMessageType {
	MSG_none = 0,
	MSG_createSharedTexture = 1,
	MSG_getSharedTexture = 2,
	MSG_getMessageQueue = 3,
	MSG_sendQueueMessage = 4,
	MSG_receiveQueueMessage = 5,
	MSG_sendQueueData = 6,
	MSG_receiveQueueData = 7,
	MSG_getSemaphore = 8
};

class NetInterface : public VisLinkAPI {
public:
	virtual ~NetInterface() {}
	
	void sendMessage(SOCKET s, NetMessageType type, const unsigned char *data, int len);
	NetMessageType receiveMessage(SOCKET s, int& len);
	int sendData(SOCKET s, const unsigned char *buf, int len);
	int receiveData(SOCKET s, unsigned char *buf, int len);
	int sendfd(SOCKET socket, int fd);
	int recvfd(SOCKET socket);

	/// return 0 for big endian, 1 for little endian.
	/// http://stackoverflow.com/questions/12791864/c-program-to-check-little-vs-big-endian
	/*static bool isLittleEndian() {
		volatile uint32_t i=0x01234567;
		return (*((uint8_t*)(&i))) == 0x67;
	}

	static void packInt(unsigned char *bytePtr, int32_t toPack) {
		unsigned char *p = (unsigned char *) &toPack;
		for (int i=0;i<NET_SIZEOFINT;i++) {
			int index = i;
			if (!isLittleEndian()) {
				index = NET_SIZEOFINT - i - 1;
			}
			bytePtr[i] = p[index];
		}
	}

	static int32_t unpackInt(unsigned char *bytePtr) {
		int toReturn = 0;
		unsigned char *p = (unsigned char *) &toReturn;
		for (int i=0;i<NET_SIZEOFINT;i++) {
			int index = i;
			if (!isLittleEndian()) {
				index = NET_SIZEOFINT - i - 1;
			}
			p[i] = bytePtr[index];
		}
		return toReturn;
	}*/
};

}

#endif
