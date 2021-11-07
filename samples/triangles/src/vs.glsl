#version 450

vec3 vertices[] = {
                    vec3(-0.5f, -0.5f, 0.0f),
                    vec3( 0.5f, -0.5f, 0.0f),
                    vec3( 0.0f,  0.0f, 0.0f)};

vec3 colors[] = {
                    vec3(1.0f, 0.0f, 0.0f),
                    vec3(0.0f, 1.0f, 0.0f),
                    vec3(0.0f, 0.0f, 1.0f)};

layout(location = 0) out vec3 vertexColor;

void main ()
{
    gl_Position = vec4(vertices[gl_VertexIndex], 1.0f);
    vertexColor = colors[gl_VertexIndex];
}
