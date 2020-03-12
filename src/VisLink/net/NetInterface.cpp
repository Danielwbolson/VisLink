#include "VisLink/net/NetInterface.h"

#include <iostream>

namespace vislink {


#define LOGD(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGE(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGW(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)

int NetInterface::sendfd(SOCKET socket, int fd) {
#ifdef WIN32
	return 0;
#else
    char dummy = '$';
    struct msghdr msg;
    struct iovec iov;

    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    iov.iov_base = &dummy;
    iov.iov_len = sizeof(dummy);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);//CMSG_LEN(sizeof(int));

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));

    *(int*) CMSG_DATA(cmsg) = fd;

    int ret = sendmsg(socket, &msg, 0);

    if (ret == -1) {
        LOGE("sendmsg failed with %s", strerror(errno));
    }

    return ret;
#endif
}

int NetInterface::recvfd(SOCKET socket) {
#ifdef WIN32
	return 0;
#else
    int len;
    int fd;
    char buf[1];
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *cmsg;
    char cms[CMSG_SPACE(sizeof(int))];

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = (caddr_t) cms;
    msg.msg_controllen = sizeof cms;

    len = recvmsg(socket, &msg, 0);

    if (len < 0) {
        LOGE("recvmsg failed with %s", strerror(errno));
        return -1;
    }

    if (len == 0) {
        LOGE("recvmsg failed no data");
        return -1;
    }

    cmsg = CMSG_FIRSTHDR(&msg);
    memmove(&fd, CMSG_DATA(cmsg), sizeof(int));
    return fd;
#endif
}

int NetInterface::sendData(SOCKET s, const unsigned char *buf, int len) {
  int total = 0;        // how many bytes we've sent
  int bytesleft = len;  // how many we have left to send
  int n = 0;
  while (total < len) {
#ifdef WIN32
	  n = send(s, (char *)(buf + total), bytesleft, 0);
#else
	  n = send(s, buf + total, bytesleft, 0);
#endif
    if (n == -1) { break; }
    total += n;
    bytesleft -= n;
  }
  return n==-1?-1:total; // return -1 on failure, total on success
}

int NetInterface::receiveData(SOCKET s, unsigned char *buf, int len) {
  int total = 0;        // how many bytes we've received
  int bytesleft = len; // how many we have left to receive
  int n = 0;
  while (total < len) {
#ifdef WIN32
	  n = recv(s, (char *)(buf + total), bytesleft, 0);
#else
	  n = recv(s, (char *)(buf + total), bytesleft, 0);
#endif
    if (n == -1) { break; }
    total += n;
    bytesleft -= n;
  }
  return n==-1?-1:total; // return -1 on failure, total on success
}

void NetInterface::sendMessage(SOCKET s, NetMessageType type, const unsigned char *data, int len) {
	int dataSize =  len + 1 + sizeof(int);
	unsigned char *buf = new unsigned char[dataSize+1];
	//1. add 1-byte message header
	buf[0] = type;
	// 2. add the size of the message data so receive will know how
	// many bytes to expect.
	memcpy(&buf[1], &len, sizeof(int));
	// 3. send the chars that make up the data.
	memcpy(&buf[1 + sizeof(int)], data, len);
	//4. send package
	sendData(s,buf,dataSize);
	//5. delete buffer
	delete[] buf;
}

NetMessageType NetInterface::receiveMessage(SOCKET s, int& len) {
	unsigned char type;
#ifdef WIN32
	len = recv(s, (char*)&type, 1, 0);
#else
	len = read(s, &type, 1);
#endif
	
	if (len == 0) {
		return MSG_none;
	}

#ifdef WIN32
	recv(s, (char*)&len, sizeof(int), 0);
#else
	read(s, &len, sizeof(int));
#endif

	return static_cast<NetMessageType>(type);
}

}
