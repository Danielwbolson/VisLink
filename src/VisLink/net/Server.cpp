#include "VisLink/net/Server.h"

#include <sstream>
#include <iostream>

#define BACKLOG 100

namespace vislink {

Server::Server(int listenPort, int numExpectedClients) {
  std::cout << "Starting networking" << std::endl;


#ifdef WIN32  // Winsock implementation

    WSADATA wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        std::stringstream s;
        s << "WSAStartup failed with error: " << iResult;
  		std::cout << s.str() << "Check for a problem with Windows networking." << std::endl;
        exit(1);
    }


    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    
    // Resolve the server address and port
    struct addrinfo *result = NULL;
    std::stringstream port;
    port << listenPort;
    iResult = getaddrinfo(NULL, port.str(), &hints, &result);
    if ( iResult != 0 ) {
        std::stringstream s;
        s << "WSAStartup failed with error: " << iResult;
  		std::cout << s.str() << "Check for a problem with Windows networking." << std::endl;
        WSACleanup();
        exit(1);
    }
    

    SOCKET serv_fd = INVALID_SOCKET;
    serv_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serv_fd == INVALID_SOCKET) {
  		std::cout << "cannot create a socket socket() failed" << std::endl;
        WSACleanup();
        exit(1);
    }
    
    const char yes = 1;
    if (setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
  		std::cout << "setsockopt() failed. Check for a problem with networking." << std::endl;
        WSACleanup();
        exit(1);
    }
    
    if (::bind(serv_fd, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
  		std::cout << "bind() failed. Check for a problem with networking." << std::endl;
        WSACleanup();
        closesocket(serv_fd);
        exit(1);
    }
    
    if (listen(serv_fd, BACKLOG) == SOCKET_ERROR) {
  		std::cout << "listen() failed. Check for a problem with networking." << std::endl;
        WSACleanup();
        exit(1);
    }
    
    std::cout << "listening for client connection(s) on port " << listenPort << "..." << std::endl;
    
    socklen_t client_len;
    struct sockaddr_in client_addr;
    SOCKET client_fd;
    
    int numConnected = 0;
    while (numConnected < numExpectedClients) {
        client_len = sizeof(client_addr);
        client_fd = accept(serv_fd, (struct sockaddr *) &client_addr, &client_len);
        if (client_fd == INVALID_SOCKET) {
  			std::cout << "accept() failed. Check for a problem with networking." << std::endl;
            WSACleanup();
            exit(1);
        }
        
        // Disable Nagle's algorithm on the client's socket
        setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
        
        numConnected++;
        
        char clientname[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, clientname, sizeof(clientname));
        
        std::stringstream s;
        s << "Received connection " << numConnected << " of " << numExpectedClients << " from " << clientname;
        std::cout << s.str() << std::endl;
        
        clientSocketFDs.push_back(client_fd);
    }

	std::cout << "Established all expected connections." << std::endl;


#else  // BSD sockets implementation

    int yes=1;

    int serv_fd, client_sockfd;
    int server_len;
    struct sockaddr_un serv_addr;
    
    unlink("server_socket");
    if ((serv_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
  		std::cout << "cannot create a socket. socket(AF_INET, SOCK_STREAM, 0) failed" << std::endl;
        exit(1);
    }


    if (setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
  		std::cout << "setsockopt() failed. Check for a problem with networking." << std::endl;
        exit(1);
    }
    
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, "server_socket");
    server_len = sizeof(serv_addr);
    
    if (::bind(serv_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
  		std::cout << "bind() failed. Check for a problem with networking." << std::endl;
        close(serv_fd);
        exit(1);
    }

    if (listen(serv_fd, BACKLOG) == -1) {
  		std::cout << "listen() failed. Check for a problem with networking." << std::endl;
        exit(1);
    }

    std::cout << "listening for client connection(s) on port " << listenPort << "..." << std::endl;
    
    socklen_t client_len;
    struct sockaddr_un client_addr;
    int client_fd;
    
    int numConnected = 0;
    while (numConnected < numExpectedClients) {
        client_len = sizeof(client_addr);
        client_fd = accept(serv_fd, (struct sockaddr *) &client_addr, &client_len);
        if (client_fd == -1) {
  			std::cout << "accept() failed. Check for a problem with networking." << std::endl;
            exit(1);
        }
        
        // Disable Nagle's algorithm on the client's socket
        //setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
        
        numConnected++;
        
        //char clientname[INET_ADDRSTRLEN];
        //inet_ntop(AF_INET, &client_addr.sin_addr, clientname, sizeof(clientname));
        
        std::stringstream s;
        s << "Received connection " << numConnected << " of " << numExpectedClients << " from " << "clientname";
        std::cout << s.str() << std::endl;
        
        clientSocketFDs.push_back(client_fd);
    }

	std::cout << "Established all expected connections." << std::endl;

#endif
}


#define LOGD(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGE(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGW(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)

int Server::sendfd(int fd) {
    for (int f = 0; f < clientSocketFDs.size(); f++) {
        sendfd(clientSocketFDs[f], fd);
    }

    return 0;
}

int Server::sendfd(SOCKET socket, int fd) {

  std::cout << "sendfd " << std::endl;
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

    std::cout << CMSG_LEN(sizeof(int)) << " " << CMSG_SPACE(sizeof(int)) << " " << sizeof(cmsgbuf) << std::endl;

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));

    *(int*) CMSG_DATA(cmsg) = fd;

    std::cout << cmsg << std::endl;

    int ret = sendmsg(socket, &msg, 0);

    if (ret == -1) {
        LOGE("sendmsg failed with %s", strerror(errno));
    }

    return ret;
}

Server::~Server() {
	std::cout << "Closing all sockets." << std::endl;
	for (std::vector<SOCKET>::iterator i=clientSocketFDs.begin(); i < clientSocketFDs.end(); i++) {
	#ifdef WIN32
	  closesocket(*i);
	#else
	  close(*i);
	#endif
	}

	#ifdef WIN32
	  WSACleanup();
	#endif
}

}
