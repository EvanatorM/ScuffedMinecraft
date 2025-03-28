#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D tex;

const vec3 ambient = vec3(.5);
const vec3 lightDirection = vec3(0.8, 1, 0.7);

const vec3 normal = vec3( 0, -1,  0);

void main()
{
	vec3 lightDir = normalize(-lightDirection);

	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1);

	vec4 result = vec4(ambient + diffuse, 1.0);

	vec4 texResult = texture(tex, TexCoord);
	if (texResult.a == 0)
		discard;
	FragColor = texResult * result;
}