#include "VisLink/net/Client.h"

#include <sstream>
#include <iostream>

namespace vislink {

// get sockaddr, IPv4 or IPv6:
void *get_in_addr2(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

Client::Client(const std::string &serverIP, int serverPort)
{
  std::cout <<"VRNetClient connecting..." << std::endl;


  std::stringstream port;
  port << serverPort;

#ifdef WIN32  // WinSock implementation

  WSADATA wsaData;
  SOCKET sockfd = INVALID_SOCKET;
  struct addrinfo hints, *servinfo, *p;
  int rv;

  rv = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (rv != 0) {
    std::stringstream s;
    s << "WSAStartup failed with error: " << rv;
    std::cout <<s.str() << "Check for a problem with Windows networking." << std::endl;
    exit(1);
  }

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;



  if ((rv = getaddrinfo(serverIP.c_str(), port.str().c_str(), &hints, &servinfo)) != 0) {
    std::stringstream s;
    s << "getaddrinfo() failed with error: " << rv;
    std::cout <<s.str() << "Check for a problem with Windows networking." << std::endl;
    WSACleanup();
    exit(1);
  }

  //This is a temporary fix to ensure the client can connect and that the connection is not refused
  p = NULL;
  while(p == NULL){
    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
	      std::stringstream s;
        s << "socket() failed with error " << WSAGetLastError() << "; will retry.";
        std::cout <<s.str() << std::endl;
        continue;
      }

      if (connect(sockfd, p->ai_addr, (int)p->ai_addrlen) == SOCKET_ERROR) {
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
	      std::stringstream s;
        s << "connect() to server socket failed; will retry.";
        std::cout <<s.str() << std::endl;
        continue;
      }

      break;
    }
  }

  if (p == NULL) {
    std::cout <<"VRNetClient failed to connect -- exiting." << "Check for a problem with Windows networking." << std::endl;
    exit(2);
  }

  //inet_ntop(p->ai_family, get_in_addr2((struct sockaddr *)p->ai_addr), s, sizeof s);
  //printf("client: connecting to %s\n", s);
  std::cout <<"VRNetClient connected." << std::endl;

  freeaddrinfo(servinfo); // all done with this structure

  // Disable Nagle's algorithm
  char value = 1;
  setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

  socketFD = sockfd;


#else  // BSD sockets implementation
/*//"server_socket"
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];
  char problemString[50];

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(serverIP.c_str(), port.str().c_str(), &hints, &servinfo)) != 0) {
    std::stringstream s;
    s << "getaddrinfo() failed with error: " << rv;
    std::cout <<s.str() << "Check for a problem with networking." << std::endl;
    exit(1);
  }

  //This is a temporary fix to ensure the client can connect and that the connection is not refused
  p = NULL;
  while(p == NULL){
    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        std::stringstream s;
        s << "socket() failed; will retry.";
        std::cout <<s.str() << std::endl;
        continue;
      }

      if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        std::stringstream s;
        s << "client (pid=" << getpid() << "): connect() to server socket failed; will retry.";
        std::cout <<s.str() << std::endl;
        continue;
      }

      break;
    }
  }
  if (p == NULL) {
    std::cout <<"VRNetClient failed to connect -- exiting." << "Check for a problem with networking." << std::endl;
    exit(2);
  }

  inet_ntop(p->ai_family, get_in_addr2((struct sockaddr *)p->ai_addr), s, sizeof(s));
  std::cout <<"VRNetClient connected to " + std::string(s) << std::endl;

  freeaddrinfo(servinfo); // all done with this structure

  // Disable Nagle's algorithm
  char value = 1;
  setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

  socketFD = sockfd;
  */

    int sockfd;
    int len;
    struct sockaddr_un address;
    int result;

/*  Create a socket for the client.  */

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

/*  Name the socket, as agreed with the server.  */

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, "server_socket");
    len = sizeof(address);

/*  Now connect our socket to the server's socket.  */
    result = -1;

    while(result < 0) {
      result = connect(sockfd, (struct sockaddr *)&address, len);

      if(result == -1) {
          perror("oops: client1");
          //exit(1);
      }
    }

    socketFD = sockfd;
#endif

}


#define LOGD(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGE(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGW(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)

int Client::recvfd() {
  return recvfd(socketFD);
}

int Client::recvfd(int socket) {
  std::cout << "recvfd " << std::endl;
    int len;
    int fd;
    char buf[1];
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *cmsg;
    char cms[CMSG_SPACE(sizeof(int))];
    std::cout << CMSG_SPACE(sizeof(int)) << std::endl;

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = (caddr_t) cms;
    msg.msg_controllen = sizeof cms;

#ifdef  HAVE_MSGHDR_MSG_CONTROL
    std::cout << "has mesghdr control" << std::endl;
#else
    std::cout << "not has mesghdr control" << std::endl;
#endif

    std::cout << sizeof cms  << " " << msg.msg_control << " " << msg.msg_controllen << std::endl;

    len = recvmsg(socket, &msg, 0);
    std::cout << sizeof cms  << " " << msg.msg_control << " " << msg.msg_controllen << std::endl;

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
}


Client::~Client()
{
  std::cout <<"Client closing socket." << std::endl;
#ifdef WIN32
  closesocket(socketFD);
  WSACleanup();
#else
  close(socketFD);
#endif
}

}
