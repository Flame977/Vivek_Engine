#version 450

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform MaterialUBO
{
    float shininess;
    float specularStrength;
} material;

layout(set = 0, binding = 1) uniform sampler2D shadowMap;

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragLightSpace;

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
    mat4 lightSpace;

    ivec4 lightInfo;
    vec4 cameraPos;

    Light lights[MAX_LIGHTS];
} ubo;




vec2 ComputeReceiverPlaneDepthBias(vec3 projCoords)
{
    vec2 biasUV;
    vec3 dx = dFdx(projCoords);
    vec3 dy = dFdy(projCoords);
    biasUV.x = dy.y * dx.z - dx.y * dy.z;
    biasUV.y = dx.x * dy.z - dy.x * dx.z;
    biasUV /= (dx.x * dy.y - dx.y * dy.x);
    return biasUV;
}


float CalculateShadow(vec4 lightSpacePos, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float shadow = 0.0;

    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
        }

    shadow /= 9.0;
    return shadow;
}





void main()
{
    vec3 norm = normalize(fragNormal);
    vec3 albedo = texture(texSampler, fragUV).rgb;
    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);
    vec3 result = vec3(0.0);


    for (int i = 0; i < ubo.lightInfo.x; i++)
    {
        Light light = ubo.lights[i];

        vec3 lightDir = normalize(-light.direction.xyz);

        float attenuation = 1.0;
        float intensity = light.color.w;

        // SPOT LIGHT
        if (light.position.w == 2.0)
        {
            vec3 toLight = light.position.xyz - fragPos;

            float dist = length(toLight);

            lightDir = normalize(toLight);

            float range = light.params.x;

            attenuation = 1.0 / (1.0 + (dist * dist) / (range * range));

            vec3 spotDir = normalize(-light.direction.xyz);

            float theta = dot(lightDir, spotDir);

            float inner = light.params.y;
            float outer = light.params.z;

            intensity = smoothstep(outer, inner, theta) * light.color.w;
        }

        // POINT LIGHT
        else if (light.position.w == 1.0)
        {
            vec3 toLight = light.position.xyz - fragPos;

            float dist = length(toLight);

            lightDir = normalize(toLight);

            float range = light.params.x;

            attenuation = 1.0 / (1.0 + (dist * dist) / (range * range));

            float fade = clamp(1.0 - (dist / range), 0.0, 1.0);

            attenuation *= fade;
        }

        // DIFFUSE LIGHTING
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * light.color.rgb * intensity * attenuation;

        // SPECULAR (Blinn-Phong)
        vec3 halfDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfDir), 0.0), material.shininess);
        vec3 specular = spec * light.color.rgb * intensity * attenuation * material.specularStrength;


        // SHADOWS ONLY FOR DIRECTIONAL
        if (light.position.w == 0.0)
        {
            float shadow = 0;
           
            if(diff > 0.0)
            {
                shadow = CalculateShadow(
                    fragLightSpace,
                    norm,
                    lightDir
                );
            }

            diffuse *= (1.0 - shadow);
            specular *= (1.0 - shadow);
        }

        result += diffuse + specular;
    }

    // Ambient
    vec3 ambient = 0.15 * albedo;

    vec3 finalColor = result * albedo + ambient;

    outColor = vec4(finalColor, 1.0);
}