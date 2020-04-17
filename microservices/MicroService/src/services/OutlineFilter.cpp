#include "src/services/OutlineFilter.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::string OutlineFilter::getVertexShader() {
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

std::string OutlineFilter::getFragmentShader() {
	return "#version 330 \n"
		"in vec3 col;"
		"layout(location = 0) out vec4 colorOut;"
		"uniform sampler2D inputTexture[1]; "
		"uniform vec4 filterColor;"
		""
		"void main() { "
		"   vec2 tex_offset = 1.0 / textureSize(inputTexture[0], 0);"
		"   vec4 texColor = texture(inputTexture[0], vec2(1.0-col.x,col.y));"
		"   colorOut = texColor; "
		"	vec2 uv = vec2(1.0-col.x,col.y); "
		"   float result = 0.0; "
		"   for(int x = 0; x < 3; ++x) {"
		"		for(int y = 0; y < 3; ++y) {"
		"			result += texture(inputTexture[0], uv + vec2(tex_offset.x * x, tex_offset.y * y)).a;"
		"		}"
		"	}"
		"   if (result > 0.0 && result < 8.9999) { colorOut = filterColor; }"
		//"   colorOut = vec4(result);"
		"}";
}

void OutlineFilter::setParameters(vislink::MessageQueue* queue, GLuint shaderProgram) {
	glm::vec4 vec = queue->receiveObject<glm::vec4>();
	GLint loc = glGetUniformLocation(shaderProgram, "filterColor");
	glUniform4f(loc, vec.r, vec.g, vec.b, vec.a);
}