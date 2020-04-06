#ifndef VISLINK_SYNC_SEMAPHORE_H_
#define VISLINK_SYNC_SEMAPHORE_H_

#ifdef WIN32
#include <windows.h>
#endif

#include <vector>

namespace vislink {

struct Semaphore;

class SyncMethod {
public:
	virtual ~SyncMethod() {}
	virtual void signal(const Semaphore& semaphore) = 0;
	virtual void waitForSignal(const Semaphore& semaphore) = 0;
};

struct Semaphore {
	Semaphore() : method(NULL) {}
	Semaphore(const Semaphore& sem) {
		*this = sem;
	}
	Semaphore& operator=(const Semaphore& sem) {
		this->id = sem.id;
		this->externalHandle = sem.externalHandle;
		this->deviceIndex = sem.deviceIndex;
		this->method = NULL;
		return *this;
	}
	~Semaphore() {
		if (method) {
			delete method;
		}
	}

	unsigned int id;
	int deviceIndex;
#ifdef WIN32
	HANDLE externalHandle;
#else
	unsigned int externalHandle;
#endif

	void signal() { if (method) { method->signal(*this); }}
	void waitForSignal() { if (method) { method->waitForSignal(*this); }}

	void setSyncMethod(SyncMethod* method) {
		if (this->method) {
			delete this->method;
		}
		this->method = method;
	}
	
private:
	SyncMethod* method;
};

class OpenGLSemaphore {
public:
	virtual ~OpenGLSemaphore() {}
	virtual unsigned int getId() const = 0;
	virtual const Semaphore& getSemaphore() const = 0;
};

OpenGLSemaphore* createOpenGLSemaphore(const Semaphore& semaphore, ProcLoader* procLoader = NULL);

class OpenGLSync : public SyncMethod {
public:
	void signal(const Semaphore& semaphore);
	void waitForSignal(const Semaphore& semaphore);

	void addTexture(unsigned int id, unsigned int layout);
	void write(const Texture& tex);
	void read(const Texture& tex);
private:
	std::vector<unsigned int> textures;
	std::vector<unsigned int> layouts;
};

}

#endif