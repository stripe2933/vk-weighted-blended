#version 450

const vec3 lightColor = vec3(1.0);
const float specularStrength = 0.5;

layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec4 fragInstanceColor;

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform PushConstant {
    mat4 projectionView;
    vec3 viewPosition;
} pc;

layout (early_fragment_tests) in;

void main() {
    // Diffuse.
    vec3 viewDir = normalize(pc.viewPosition - fragPosition);
    vec3 lightDir = viewDir;
    float diff = max(dot(lightDir, fragNormal), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular.
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(fragNormal, halfwayDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;

    outColor = vec4((diffuse + specular) * fragInstanceColor.rgb, 1.0);
}