#ifndef VISLINK_SERVER_H_
#define VISLINK_SERVER_H_

#include <string>
#include <vector>
#include <queue>
#include <map>

#include "VisLink/net/NetInterface.h"
#include "VisLink/impl/VisLinkAPIImpl.h"

namespace vislink {

class ServerMessageQueue {
public:
	ServerMessageQueue(NetInterface* net, int id) : net(net), id(id), currentMessageId(0), sending(false) {}
	virtual ~ServerMessageQueue() {}
	int getId() { return id; }
	void pushClient(SOCKET clientFD);
	void pushMessage();
	void pushMessageData(SOCKET sd);
private:
	struct Message {
		Message(int id) : id(id), data(NULL) {}
		bool isStart() { return !data; }

		int id;
		unsigned char* data;
		int len;
	};

	void sendData();

	NetInterface* net;
	std::queue<SOCKET> clientWaitQueue;
	std::queue<Message> sendDataQueue;
	int id;
	int currentMessageId;
	bool sending;
};

class Server : public NetInterface {
public:
	Server(int listenPort = 3457, int numExpectedClients = 0);
	~Server();

	void createSharedTexture(const std::string& name, const TextureInfo& info, int deviceIndex) { 
		impl.createSharedTexture(name, info, deviceIndex); 
	}
	Texture getSharedTexture(const std::string& name, int deviceIndex) { return impl.getSharedTexture(name, deviceIndex); }

	MessageQueue* getMessageQueue(const std::string& name) { 
		/*std::map<std::string,int>::iterator it = messageQueueMap.find(name);
		int id = messageQueues.size();
		if (it != messageQueueMap.end()) {
			id = it->second;
		}
		else {
			MessageQueue* queue = impl.getMessageQueue(name);
			messageQueueMap[name] = id;
			messageQueues.push_back(queue);
		}
		return messageQueues[id];*/
		return NULL;
	}

	void service();

private:
	ServerMessageQueue* getServerMessageQueue(const std::string& name) { 
		std::map<std::string,int>::iterator it = messageQueueMap.find(name);
		int id = messageQueues.size();
		if (it != messageQueueMap.end()) {
			id = it->second;
		}
		else {
			ServerMessageQueue* queue = new ServerMessageQueue(this, id);
			messageQueueMap[name] = id;
			messageQueues.push_back(queue);
		}
		return messageQueues[id];
	}

	ServerMessageQueue* getQueueFromMessage(SOCKET sd);

	SOCKET serverSocketFD;
	std::vector<SOCKET> clientSocketFDs;
	VisLinkAPIImpl impl;
	fd_set readfds;
	std::map<std::string, int> messageQueueMap;
	std::vector<ServerMessageQueue*> messageQueues;
};

}

#endif