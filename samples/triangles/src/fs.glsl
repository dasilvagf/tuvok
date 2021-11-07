#version 450

layout(location = 0) in  vec3 vertexColor;
layout(location = 0) out vec4 rgbaColor;

void main()
{
    rgbaColor = vec4(vertexColor, 1.0f);
}
