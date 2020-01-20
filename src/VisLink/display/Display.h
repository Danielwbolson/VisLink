#ifndef VISLINK_DISPLAY_DISPLAY_H_
#define VISLINK_DISPLAY_DISPLAY_H_

#include "VisLink/VisLinkAPI.h"

namespace vislink {

class Display {
public:
	virtual void init() = 0;
	virtual void render() = 0;
	virtual void finish() = 0;
	virtual void display() = 0;
	virtual void useContext() = 0;
	virtual void releaseContext() = 0;
};

}

#endif