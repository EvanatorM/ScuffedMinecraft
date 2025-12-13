#version 450 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in float LightMultiplier;
in float SkyLightMultiplier;

uniform sampler2D tex;

float ambientStrength = 0.2;
vec3 ambientColor = vec3(1.0);

void main()
{
    // Ambient lighting
    vec3 ambient = ambientStrength * ambientColor;

    // Light levels
    vec3 lighting = vec3(1.0) * LightMultiplier;
    vec3 skyLighting = vec3(1.0) * SkyLightMultiplier;
    lighting += skyLighting;

    // Calculate result
    vec3 result = min(ambient + lighting, 1.0);

    vec4 texResult = texture(tex, TexCoord);
	FragColor = texResult * vec4(result, 1.0);
}