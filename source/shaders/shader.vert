#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat3 transform;
} transformData;
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 out_color;

void main() {
    gl_Position = vec4(vec3(position*transformData.transform), 1.0);
    out_color = color;
}
