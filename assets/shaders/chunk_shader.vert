#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in float aLightLevel;
layout(location = 4) in float aSkyLightLevel;

out vec2 TexCoord;
out vec3 Normal;
out float LightMultiplier;
out float SkyLightMultiplier;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

const float LIGHT_LEVEL_MULTIPLIER = 1.0 / 15.0;

void main()
{
    gl_Position = proj * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    Normal = aNormal;
    LightMultiplier = aLightLevel * LIGHT_LEVEL_MULTIPLIER;
    SkyLightMultiplier = aSkyLightLevel * LIGHT_LEVEL_MULTIPLIER;
}