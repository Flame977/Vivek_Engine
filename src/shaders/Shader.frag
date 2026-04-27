#version 450

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;


#define MAX_LIGHTS 16

struct Light
{
    vec4 position;
    vec4 direction;
    vec4 color;      // w component of this vecotor is the intensity of the light !!!
    vec4 params;
};

layout(set = 0, binding = 0) uniform CameraUBO
{
    mat4 view;
    mat4 proj;

    ivec4 lightInfo;

    Light lights[MAX_LIGHTS];
} ubo;


void main()
{
    vec3 norm = normalize(fragNormal);
    vec3 albedo = texture(texSampler, fragUV).rgb;
    
    vec3 result = vec3(0.0);
    
    for (int i = 0; i < ubo.lightInfo.x; i++)
    {
        Light light = ubo.lights[i];

        vec3 lightDir = normalize(-light.direction.xyz);
        
        float attenuation = 1.0;
        float intensity = light.color.w;

        if (light.position.w == 2.0) // SPOT LIGHT
        {
            vec3 toLight = light.position.xyz - fragPos;
            float dist = length(toLight);
        
            lightDir = normalize(toLight);
        
            // attenuation (distance)
            float range = light.params.x;
            attenuation = 1.0 / (1.0 + (dist * dist) / (range * range));
        
            // spotlight cone
            vec3 spotDir = normalize(-light.direction.xyz);
        
            float theta = dot(lightDir, spotDir);
        
            float inner = light.params.y;
            float outer = light.params.z;
        
            intensity = smoothstep(outer, inner, theta) * light.color.w;
        
        }


        float diff = max(dot(norm, lightDir), 0.0);
    
        vec3 diffuse = diff * light.color.rgb * intensity * attenuation;
    
        result += diffuse;
    }
    
    // Ambient
    vec3 ambient = 0.2 * albedo;
    
    vec3 finalColor = result * albedo + ambient;
    
    outColor = vec4(finalColor, 1.0);

}
