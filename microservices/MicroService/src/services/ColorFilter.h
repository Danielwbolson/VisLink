#ifndef COLORFILTER_H_
#define COLORFILTER_H_

#include "src/MicroService.h"

class ColorFilter : public FrameBufferService {
public:
	ColorFilter(const std::string& name, vislink::VisLinkAPI* api) : FrameBufferService(name, api) {}

	std::string getVertexShader();
	std::string getFragmentShader();
	void setParameters(vislink::MessageQueue* queue, GLuint shaderProgram);
};

#endif