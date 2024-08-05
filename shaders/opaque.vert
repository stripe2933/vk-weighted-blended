#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inInstancePosition;
layout (location = 3) in vec4 inInstanceColor;

layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec4 fragInstanceColor;

layout (push_constant) uniform PushConstant {
    mat4 projectionView;
    vec3 viewPosition;
} pc;

vec3 toEuclideanCoord(vec4 homogeneousCoord){
    return homogeneousCoord.xyz / homogeneousCoord.w;
}

void main() {
    fragPosition = inPosition + inInstancePosition;
    fragNormal = inNormal;
    fragInstanceColor = inInstanceColor;

    gl_Position = pc.projectionView * vec4(fragPosition, 1.0);
}