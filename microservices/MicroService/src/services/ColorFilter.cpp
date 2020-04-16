#include "src/services/ColorFilter.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::string ColorFilter::getVertexShader() {
	return "#version 330 \n"
		"layout(location = 0) in vec3 position; "
		"layout(location = 1) in vec3 normal; "
		"layout(location = 2) in vec3 color; "
		"out vec3 col;"
		""
		"void main() { "
		"   gl_Position = vec4(position, 1.0); "
		"   col = color; "
		"}";
}

std::string ColorFilter::getFragmentShader() {
	return "#version 330 \n"
		"in vec3 col;"
		"layout(location = 0) out vec4 colorOut;"
		"uniform sampler2D inputTexture[1]; "
		"uniform vec4 filterColor;"
		""
		"void main() { "
		"   vec4 texColor = texture(inputTexture[0], vec2(1.0-col.x,col.y));"
		"   colorOut = texColor*filterColor; "
		//"   colorOut = vec4(1.0,0,0,1); "
		"}";
}

void ColorFilter::setParameters(vislink::MessageQueue* queue, GLuint shaderProgram) {
	glm::vec4 vec = queue->receiveObject<glm::vec4>();
	GLint loc = glGetUniformLocation(shaderProgram, "filterColor");
	glUniform4f(loc, vec.r, vec.g, vec.b, vec.a);
}