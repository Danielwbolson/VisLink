#ifndef THRESHOLDFILTER_H_
#define THRESHOLDFILTER_H_

#include "src/MicroService.h"

class ThresholdFilter : public FrameBufferService {
public:
	ThresholdFilter(const std::string& name, vislink::VisLinkAPI* api) : FrameBufferService(name, api) {}

	std::string getVertexShader();
	std::string getFragmentShader();
	void setParameters(vislink::MessageQueue* queue, GLuint shaderProgram);
};

#endif
