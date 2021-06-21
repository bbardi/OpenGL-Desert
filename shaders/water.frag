#version 400 core

in vec4 clipSpaceCoord;

out vec4 out_Color;

uniform sampler2D reflection;
uniform sampler2D refraction;

void main(void) {

	vec2 ndc = (clipSpaceCoord.xy/clipSpaceCoord.w)/2.0 + 0.5;

	vec2 refractCoord = vec2(ndc.x,ndc.y);
	vec2 reflectCoord = vec2(ndc.x,-ndc.y);
	vec4 reflectColor = texture(reflection,reflectCoord);
	vec4 refractColor = texture(refraction,refractCoord);
	out_Color = mix(reflectColor,refractColor,0.50);
}