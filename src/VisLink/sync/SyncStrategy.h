#ifndef VISLINK_SYNC_SYNC_STRATEGY_H_
#define VISLINK_SYNC_SYNC_STRATEGY_H_

#include <vector>
#include <typeinfo>
#include "VisLink/sync/Semaphore.h"
#include "VisLink/queue/MessageQueue.h"
#include <string>

namespace vislink {

class SyncStrategy {
public:
	virtual ~SyncStrategy() {}

	virtual void signal() = 0;
	virtual void waitForSignal() = 0;

	template<typename T>
	void addObject(T val) {
		static const std::type_info& type = typeid(T);
		addObjectImpl(type, &val);
	}

protected:
	template<typename T>
	bool isType(const std::type_info& type) {
		static const std::type_info& testType = typeid(T);
		return type == testType;
	}

	static void addObjectImpl(SyncStrategy* sync, const std::type_info& type, void* val) {
		sync->addObjectImpl(type, val);
	}
	virtual void addObjectImpl(const std::type_info& type, void* val) {}
};

class EmptySyncStrategy : public SyncStrategy {
public:
	virtual void signal() {}
	virtual void waitForSignal() {}
};

class CompositeSyncStrategy : public SyncStrategy {
public:
	void addChild(SyncStrategy* strategy) {
		children.push_back(strategy);
	}

	void signal() {
		for (int f = 0; f < children.size(); f++) {
			children[f]->signal();
		}
	}

	void waitForSignal() {
		for (int f = children.size()-1; f >= 0; f--) {
			children[f]->waitForSignal();
		}
	}

protected:
	void addObjectImpl(const std::type_info& type, void* val) {
		for (int f = children.size()-1; f >= 0; f--) {
			SyncStrategy::addObjectImpl(children[f], type, val);
		}
	}

private:
	std::vector<SyncStrategy*> children;
};

class PrintSyncStrategy : public SyncStrategy {
public:
	PrintSyncStrategy(const std::string& name) : name(name) {}

	void signal() {
		std::cout << name << ": signal()" << std::endl;
	}

	void waitForSignal() {
		std::cout << name << ": waitForSignal()" << std::endl;
	}

private:
	std::string name;
};

class QueueSyncStrategy : public SyncStrategy {
public:
	QueueSyncStrategy() : queue(NULL) {}
	void signal() {
		//std::cout << queue << std::endl;
		//exit(0);
		if (queue) {
			queue->sendMessage();
		}
		else {
			std::cout << "no queue" << std::endl;
			exit(0);
		}
	}

	void waitForSignal() {
		if (queue) {
			queue->waitForMessage();
		}
		else {
			std::cout << "no queue" << std::endl;
			exit(0);
		}
	}

protected:
	void addObjectImpl(const std::type_info& type, void* val) {
		if (isType<MessageQueue*>(type)) {
			queue = *static_cast<MessageQueue**>(val);
		}
	}

private:
	MessageQueue* queue;
};

struct ReadTexture {
	ReadTexture(Texture tex) : tex(tex) {}
	Texture tex;
};

struct WriteTexture {
	WriteTexture(Texture tex) : tex(tex) {}
	Texture tex;
};

class OpenGLSemaphoreSync : public SyncStrategy {
public:
	OpenGLSemaphoreSync() : hasSemaphore(false) {}
	void signal();
	void waitForSignal();

	void addTexture(unsigned int id, unsigned int layout);
	void write(const Texture& tex);
	void read(const Texture& tex);
	
protected:
	void addObjectImpl(const std::type_info& type, void* val) {
		if (isType<Semaphore>(type)) {
			semaphore = *static_cast<Semaphore*>(val);
			hasSemaphore = true;			//exit(0);
		}
		else if (isType<ReadTexture>(type)) {
			Texture tex = (*static_cast<ReadTexture*>(val)).tex;
			read(tex);
			//std::cout << "tex " << tex.id << std::endl;
			//exit(0);
		}
		else if (isType<WriteTexture>(type)) {
			Texture tex = (*static_cast<WriteTexture*>(val)).tex;
			write(tex);
			//std::cout << "tex " << tex.id << std::endl;
			//exit(0);
		}
	}


public:
	bool hasSemaphore;
	Semaphore semaphore;	
	std::vector<unsigned int> textures;
	std::vector<unsigned int> layouts;
};

class BasicOpenGLSync : public CompositeSyncStrategy {
public:
	BasicOpenGLSync() {
		addChild(new OpenGLSemaphoreSync());
		addChild(new QueueSyncStrategy());
	}
};

}

#endif