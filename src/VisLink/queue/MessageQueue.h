#ifndef VISLINK_QUEUE_MESSAGE_QUEUE_H_
#define VISLINK_QUEUE_MESSAGE_QUEUE_H_

#include <iostream>

namespace vislink {

class MessageQueue {
public:
	virtual ~MessageQueue() {}
	virtual int getId() = 0;
	virtual void waitForMessage() = 0;
	virtual void sendMessage() = 0;
	virtual int sendData(const unsigned char *buf, int len) = 0;
	virtual int receiveData(unsigned char *buf, int len) = 0;

	template<typename T>
	void sendObject(T obj) {
		sendData((unsigned char*)& obj, sizeof(T));
	}

	template<typename T>
	T receiveObject() {
		T obj;
		receiveData((unsigned char*)& obj, sizeof(T));
		return obj;
	}
};

}


#endif