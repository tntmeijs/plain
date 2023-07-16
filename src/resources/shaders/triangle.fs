#version 450

layout(location = 0) in vec3 inputVertexColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(inputVertexColor, 1.0);
}
