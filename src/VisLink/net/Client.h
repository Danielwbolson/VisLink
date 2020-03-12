#ifndef VISLINK_CLIENT_H_
#define VISLINK_CLIENT_H_

#include <string>
#include <iostream>

#include "VisLink/net/NetInterface.h"
#include "VisLink/VisLinkAPI.h"

namespace vislink {

class ClientMessageQueue : public MessageQueue {
public:
	ClientMessageQueue(NetInterface* net, SOCKET socketFD, int id);
	virtual ~ClientMessageQueue() {}
	int getId();
	void waitForMessage();
	void sendMessage();
	int sendData(const unsigned char *buf, int len);
	int receiveData(unsigned char *buf, int len);
private:
	NetInterface* net;
	SOCKET socketFD;
	int id;
};

class Client : public NetInterface  {
public:
	Client(const std::string &serverIP = "127.0.0.1", int serverPort = 3457);
	~Client();

	virtual void createSharedTexture(const std::string& name, const TextureInfo& info, int deviceIndex) {
	    sendMessage(socketFD, MSG_createSharedTexture, (const unsigned char*)name.c_str(), sizeof(name.c_str()));
	    sendData(socketFD, (unsigned char*)& deviceIndex, sizeof(int));
	    sendData(socketFD, (unsigned char*)& info, sizeof(TextureInfo));
	}

	virtual Texture getSharedTexture(const std::string& name, int deviceIndex) { 
	    sendMessage(socketFD, MSG_getSharedTexture, (const unsigned char*)name.c_str(), sizeof(name.c_str()));
	    sendData(socketFD, (unsigned char*)& deviceIndex, sizeof(int));
	    Texture tex;
#ifdef WIN32
		//HANDLE currentProcess = GetCurrentProcess();
		//sendData(socketFD, (unsigned char*)&currentProcess, sizeof(HANDLE));
		int pid = GetCurrentProcessId();
		receiveData(socketFD, (unsigned char*)& pid, sizeof(int));
		std::cout << "Pid client: " << pid << std::endl;
		HANDLE serverProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pid);
		
		HANDLE externalHandleDup;
		receiveData(socketFD, (unsigned char*)& tex, sizeof(tex));
		std::cout << tex.externalHandle << " eh" << std::endl;
		DuplicateHandle(serverProcess,
			tex.externalHandle,
			GetCurrentProcess(),
			&externalHandleDup,
			0,
			FALSE,
			DUPLICATE_SAME_ACCESS);
		tex.externalHandle = externalHandleDup;
		std::cout << tex.externalHandle << " eh" << std::endl;
		//tex.externalHandle = 0;
#else
		int fd = NetInterface::recvfd(socketFD);
		receiveData(socketFD, (unsigned char*)& tex, sizeof(tex));
		tex.externalHandle = fd;
#endif
		return tex;
	}

	MessageQueue* getMessageQueue(const std::string& name) { 
		sendMessage(socketFD, MSG_getMessageQueue, (const unsigned char*)name.c_str(), sizeof(name.c_str()));
		int queueId;
		receiveData(socketFD, (unsigned char*)& queueId, sizeof(queueId));
		MessageQueue* queue = new ClientMessageQueue(this, socketFD, queueId);
		messageQueues.push_back(queue);
		return queue;
	}

private:
	std::vector<MessageQueue*> messageQueues;
	SOCKET socketFD;
};

}

#endif