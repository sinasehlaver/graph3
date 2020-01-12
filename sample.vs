#version 330 core
layout ( location = 0 ) in vec3 position;
uniform mat4 rMat;
out vec4 vertexColor;
void main()
{
	gl_Position = vec4(position, 1.0f);
	vertexColor = vec4(0.5f, 0.0f, 0.7f, 1.0f);
}
