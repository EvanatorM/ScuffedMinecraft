#version 330 core

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inTexCoords;

uniform mat4 projection;

out vec2 TexCoord;

void main()
{
	gl_Position = projection * vec4(inPos, 0.0, 1.0);
	TexCoord = inTexCoords;
}