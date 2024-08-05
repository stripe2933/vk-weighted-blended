#version 450

const vec3 lightColor = vec3(1.0);
const float specularStrength = 0.5;

layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec4 fragInstanceColor;

layout (location = 0) out vec4 outAccumulation;
layout (location = 1) out float outRevealage;

layout (push_constant) uniform PushConstant {
    mat4 projectionView;
    vec3 viewPosition;
} pc;

layout (early_fragment_tests) in;

void main() {
    // Diffuse.
    vec3 viewDir = normalize(pc.viewPosition - fragPosition);
    vec3 lightDir = viewDir;

    // Since translucent object's face can diffuse the light from the back side, we need to flip the normal if the
    // view direction is opposite to the normal.
    vec3 normal = fragNormal;
    if (dot(viewDir, normal) < 0.0){
        normal = -normal;
    }
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 color = diffuse * fragInstanceColor.rgb;

    // Weighted Blended.
    float weight = clamp(pow(min(1.0, fragInstanceColor.a * 10.0) + 0.01, 3.0) * 1e8 *
                         pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
    outAccumulation = vec4(color.rgb * fragInstanceColor.a, fragInstanceColor.a) * weight;
    outRevealage = fragInstanceColor.a;
}