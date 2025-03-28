#version 330 core

in vec2 TexCoord;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D tex;

vec3 ambient = vec3(.5);
vec3 lightDirection = vec3(0.8, 1, 0.7);

void main()
{
	vec3 lightDir = normalize(-lightDirection);

	float diff = max(dot(Normal, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1);

	vec4 result = vec4(ambient + diffuse, 1.0);

	vec4 texResult = texture(tex, TexCoord);
	if (texResult.a == 0)
		discard;
	FragColor = texResult * result;
}