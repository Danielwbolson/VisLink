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
    iResult = getaddrinfo(NULL, port.str().c_str(), &hints, &result);
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

	serverSocketFD = serv_fd;
    
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
    
    unlink("/home/dan/server_socket");
    if ((serv_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
  		std::cout << "cannot create a socket. socket(AF_INET, SOCK_STREAM, 0) failed" << std::endl;
        exit(1);
    }
    serverSocketFD = serv_fd;


    if (setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
  		std::cout << "setsockopt() failed. Check for a problem with networking." << std::endl;
        exit(1);
    }
    
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, "/home/dan/server_socket");
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

	std::cout << "Established all expected connections." << std::endl;


#endif
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

void Server::service() {
    //Adapted from https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
    //clear the socket set  
    FD_ZERO(&readfds);   
 
    //add master socket to set  
    FD_SET(serverSocketFD, &readfds);   
    int max_sd = serverSocketFD;   
         
    //add child sockets to set  
    for (int f = 0; f < clientSocketFDs.size(); f++) {
        //socket descriptor  
        int sd = clientSocketFDs[f]; 

        //if valid socket descriptor then add to read list  
        if (sd != 0) {
            FD_SET(sd , &readfds); 
        }

        //highest file descriptor number, need it for the select function   
        if(sd > max_sd) {
            max_sd = sd;
        }
    }
 
    //wait for an activity on one of the sockets , timeout is NULL ,  
    //so wait indefinitely  
#ifdef WIN32
	/*struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 200;
	int activity = select(0, &readfds, NULL, NULL, &timeout);
	if (activity == 0) {
		return;
	}*/
	int activity = select(0, &readfds, NULL, NULL, NULL);
#else
	int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
#endif
   
    if ((activity < 0) && (errno!=EINTR)) {   
        printf("select error");   
    }   
         
    //If something happened on the master socket ,  
    //then its an incoming connection  
    if (FD_ISSET(serverSocketFD, &readfds)) {   
#ifdef WIN32
		struct sockaddr_in client_addr;
#else
        struct sockaddr_un client_addr;
#endif
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(serverSocketFD, (struct sockaddr *) &client_addr, &client_len);
#ifdef WIN32
		if (client_fd == INVALID_SOCKET) {
#else
        if (client_fd == -1) {
#endif
            std::cout << "accept() failed. Check for a problem with networking." << std::endl;
            exit(1);
        }
        
        std::cout << "New Connection." << std::endl; 
       
        bool foundReplacement = false;
        for (int f = 0; f < clientSocketFDs.size(); f++) {
            if (clientSocketFDs[f] == 0) {
                clientSocketFDs[f] = client_fd;
                foundReplacement = true;
                break;
            }
        }
        if (!foundReplacement) {
            clientSocketFDs.push_back(client_fd);  
        }
    }

    //else its some IO operation on some other socket 
    for (int f = 0; f < clientSocketFDs.size(); f++) {
        //socket descriptor  
        int sd = clientSocketFDs[f]; 

        if (FD_ISSET(sd , &readfds)) {
            //Check if it was for closing , and also read the  
            //incoming message  
            int dataLength;
            //char buffer[1025]; 
            NetMessageType messageType = receiveMessage(sd, dataLength);
            if (messageType == MSG_none) { //(valread = read( sd , buffer, 1025)) == 0) {   
                //Somebody disconnected , get his details and print  
#ifdef WIN32
				struct sockaddr_in client_addr;
#else
				struct sockaddr_un client_addr;
#endif
                socklen_t client_len;
                getpeername(sd , (struct sockaddr*)&client_addr , (socklen_t*)&client_len);   
                std::cout <<"Host disconnected " << std::endl;   
                clientSocketFDs[f] = 0;
            }
            else if (messageType == MSG_createSharedTexture) {
                unsigned char* buf = new unsigned char[dataLength+1];
                receiveData(sd, buf, dataLength);
                int deviceIndex;
                receiveData(sd, (unsigned char*)& deviceIndex, sizeof(int));
                buf[dataLength] = '\0';
                std::string val(reinterpret_cast<char*>(buf));
                TextureInfo info;
                receiveData(sd, (unsigned char*)& info, sizeof(TextureInfo));
                createSharedTexture(val, info, deviceIndex);
                delete[] buf;
            }
            else if (messageType == MSG_getSharedTexture) {
                unsigned char* buf = new unsigned char[dataLength+1];
                receiveData(sd, buf, dataLength);
                int deviceIndex;
                receiveData(sd, (unsigned char*)& deviceIndex, sizeof(int));
                buf[dataLength] = '\0';
                std::string val(reinterpret_cast<char*>(buf));
                Texture tex = getSharedTexture(val, deviceIndex);
#ifdef WIN32
				int pid = GetCurrentProcessId();
				sendData(sd, (unsigned char*)& pid, sizeof(int));
				std::cout << "Pid server: " << pid << std::endl;
				//OpenProcess(PROCESS_DUP_HANDLE, FALSE, pid);
				//HANDLE externalProcess;
				//receiveData(sd, (unsigned char*)&externalProcess, sizeof(HANDLE));
				/*HANDLE externalHandleDup;

				DuplicateHandle(GetCurrentProcess(),
					tex.externalHandle,
					externalProcess,
					&externalHandleDup,
					0,
					FALSE,
					DUPLICATE_SAME_ACCESS);
				tex.externalHandle = externalHandleDup;*/
#else
                NetInterface::sendfd(sd, tex.externalHandle);
#endif
                sendData(sd, (unsigned char*)&tex, sizeof(tex));

				//int texId = 0;
				//receiveData(sd, (unsigned char*)& texId, sizeof(int));
				//std::cout << "tex id: " << texId << std::endl;
                delete[] buf;

            }
            else if (messageType == MSG_getMessageQueue) {
                unsigned char* buf = new unsigned char[dataLength+1];
                receiveData(sd, buf, dataLength);
                buf[dataLength] = '\0';
                std::string val(reinterpret_cast<char*>(buf));
                ServerMessageQueue* queue = getServerMessageQueue(val);
                int queueId = queue->getId();
                sendData(sd, (unsigned char*)& queueId, sizeof(queueId));
                delete[] buf;
            }
            else if (messageType == MSG_sendQueueMessage) {
                ServerMessageQueue* queue = getQueueFromMessage(sd);
                queue->pushMessage();
            }
            else if (messageType == MSG_receiveQueueMessage) {
                ServerMessageQueue* queue = getQueueFromMessage(sd);
                queue->pushClient(sd);
            }
            else if (messageType == MSG_sendQueueData) {
                ServerMessageQueue* queue = getQueueFromMessage(sd);
                queue->pushMessageData(sd);
            }
            else if (messageType == MSG_receiveQueueData) {
                ServerMessageQueue* queue = getQueueFromMessage(sd);
            }
            else if (messageType == MSG_getSemaphore) {
                unsigned char* buf = new unsigned char[dataLength+1];
                receiveData(sd, buf, dataLength);
                int deviceIndex;
                receiveData(sd, (unsigned char*)& deviceIndex, sizeof(int));
                buf[dataLength] = '\0';
                std::string val(reinterpret_cast<char*>(buf));
                Semaphore sem = getSemaphore(val, deviceIndex);
#ifdef WIN32
                int pid = GetCurrentProcessId();
                sendData(sd, (unsigned char*)& pid, sizeof(int));
#else
                NetInterface::sendfd(sd, sem.externalHandle);
#endif
                sendData(sd, (unsigned char*)&sem, sizeof(sem));
                delete[] buf;

            }
        }

    }
}

ServerMessageQueue* Server::getQueueFromMessage(SOCKET sd) {
    int queueId;
    receiveData(sd, (unsigned char*)& queueId, sizeof(queueId));
    return messageQueues[queueId];
}

void ServerMessageQueue::pushClient(SOCKET clientFD) {
    clientWaitQueue.push(clientFD);
    sendData();
}

void ServerMessageQueue::pushMessage() {
    currentMessageId++;
    sendDataQueue.push(Message(currentMessageId));
    sendData();
}

void ServerMessageQueue::pushMessageData(SOCKET sd) {
    int len;
    net->receiveData(sd, (unsigned char*)& len, sizeof(len));
    unsigned char* buf = new unsigned char[len];
    net->receiveData(sd, buf, len);
    Message msg = Message(currentMessageId);
    msg.data = buf;
    msg.len = len;
    sendDataQueue.push(msg);
    sendData();
}

void ServerMessageQueue::sendData() {
    while (!clientWaitQueue.empty() && !sendDataQueue.empty()) {
        
        Message msg = sendDataQueue.front();

        if (msg.isStart() && sending) {
            clientWaitQueue.pop();
        }

        sending = !clientWaitQueue.empty();

        if (sending) {
            SOCKET client = clientWaitQueue.front();

            if (msg.isStart()) {
                net->sendData(client, (unsigned char *)&msg.id, sizeof(int));
                sending = true;
            }
            else {
                net->sendData(client, msg.data, msg.len);
                delete[] msg.data;
            }

            sendDataQueue.pop();
        }
    }
}

}
