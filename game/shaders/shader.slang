struct VSInput {
    float3 inPosition;
    float2 inTexCoord;
};

struct UniformBuffer {
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

struct PushConstants {
    float4x4 model;
};
[[vk::push_constant]] PushConstants objectInfo;

struct VSOutput {
    float4 pos : SV_Position;
    float2 fragTexCoord;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(objectInfo.model, float4(input.inPosition, 1.0))));
    output.fragTexCoord = input.inTexCoord;
    return output;
}

Sampler2D texture;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_Target
{
    return texture.Sample(vertIn.fragTexCoord);
}