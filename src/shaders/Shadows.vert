#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform PushConstants
{
    mat4 model;
} pushData;

layout(set = 0, binding = 0) uniform ShadowUBO
{
    mat4 lightSpace;
} ubo;

void main()
{
    gl_Position =
        ubo.lightSpace *
        pushData.model *
        vec4(inPosition, 1.0);
}