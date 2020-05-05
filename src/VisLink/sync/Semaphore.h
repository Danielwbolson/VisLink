#ifndef VISLINK_SYNC_SEMAPHORE_H_
#define VISLINK_SYNC_SEMAPHORE_H_

#ifdef WIN32
#include <windows.h>
#endif

#include <vector>

namespace vislink {


struct Semaphore {
	Semaphore() {}
	Semaphore(const Semaphore& sem) {
		*this = sem;
	}
	Semaphore& operator=(const Semaphore& sem) {
		this->id = sem.id;
		this->externalHandle = sem.externalHandle;
		this->deviceIndex = sem.deviceIndex;
		return *this;
	}
	~Semaphore() {
	}

	unsigned int id;
	int deviceIndex;
#ifdef WIN32
	HANDLE externalHandle;
#else
	unsigned int externalHandle;
#endif

};

class OpenGLSemaphore {
public:
	virtual ~OpenGLSemaphore() {}
	virtual unsigned int getId() const = 0;
	virtual const Semaphore& getSemaphore() const = 0;
};

OpenGLSemaphore* createOpenGLSemaphore(const Semaphore& semaphore, ProcLoader* procLoader = NULL);

}

#endif