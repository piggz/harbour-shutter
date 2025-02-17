#version 330 core
layout (location = 0) in vec4 vertexIn;
out vec2 textureOut;

void main()
{
   gl_Position = vertexIn;
   textureOut = vec2(1.0,1.0);
}
