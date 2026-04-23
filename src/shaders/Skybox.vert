
#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 texDir;

layout(set = 0, binding = 0) uniform CameraUBO
{
	mat4 view;
	mat4 proj;
} camera;

void main()
{
    // Remove translation (skybox stays centered)
    mat4 viewNoTranslation = mat4(mat3(camera.view));

    vec4 pos = camera.proj * viewNoTranslation * vec4(inPosition, 1.0);

    // Push depth to far plane
    gl_Position = pos.xyww;

    texDir = inPosition;

}