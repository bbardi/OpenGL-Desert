#version 400 core

layout(location = 0) in vec2 position;

out vec4 clipSpaceCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;


void main(void) {
	clipSpaceCoord =  projection * view * model * vec4(position.x, 0.0, position.y, 1.0);
	gl_Position = clipSpaceCoord;
}