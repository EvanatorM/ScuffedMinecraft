#version 330 core


layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in int aDirection;

out vec2 TexCoord;
out vec3 Normal;

uniform float texMultiplier;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Array of possible normals based on direction
const vec3 normals[6] = vec3[6](
	vec3(0, 0, 1),  // 0
	vec3(0, 0, -1), // 1
	vec3(1, 0, 0),  // 2
	vec3(-1, 0, 0), // 3
	vec3(0, 1, 0),  // 4
	vec3(0, -1, 0)  // 5
);

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	TexCoord = aTexCoord * texMultiplier;

	Normal = normals[aDirection];
}