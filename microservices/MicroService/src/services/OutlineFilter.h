#ifndef OUTLINEFILTER_H_
#define OUTLINEFILTER_H_

#include "src/MicroService.h"

class OutlineFilter : public FrameBufferService {
public:
	OutlineFilter(const std::string& name, vislink::VisLinkAPI* api) : FrameBufferService(name, api) {}

	std::string getVertexShader();
	std::string getFragmentShader();
	void setParameters(vislink::MessageQueue* queue, GLuint shaderProgram);
};

#endif