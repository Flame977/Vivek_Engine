#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;


layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;


layout(push_constant) uniform PushConstant
{
    mat4 model;
} pushData;


#define MAX_LIGHTS 16

struct Light
{
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 params;
};

layout(set = 0, binding = 0) uniform CameraUBO
{
    mat4 view;
    mat4 proj;

    ivec4 lightInfo;

    Light lights[MAX_LIGHTS];
} camera;


void main()
{
    // World position
    vec4 worldPos = pushData.model * vec4(inPosition, 1.0);
    fragPos = worldPos.xyz;
    
    //fragNormal = normalize(mat3(pushData.model) * inNormal);

    mat3 normalMatrix = transpose(inverse(mat3(pushData.model)));
    fragNormal = normalize(normalMatrix * inNormal);

    fragUV = inUV;
    
    gl_Position = camera.proj * camera.view * worldPos;
}
