#version 450 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D tex;

float ambientStrength = 0.7;
vec3 ambientColor = vec3(1.0);
vec3 directionalLightDir = vec3(0.85, 1.0, 0.7);
vec3 directionalLightColor = vec3(0.3);

void main()
{
    // Ambient lighting
    vec3 ambient = ambientStrength * ambientColor;

    // Directional lighting
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, directionalLightDir), 0.0);
    vec3 diffuse = diff * directionalLightColor;

    // Calculate result
    vec3 result = ambient + diffuse;

    vec4 texResult = texture(tex, TexCoord);
	FragColor = texResult * vec4(result, 1.0);
}