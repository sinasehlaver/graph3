#version 330 core
layout ( location = 0 ) in vec3 position;
uniform mat4 rMat;
out vec4 vertexColor;

uniform int textureWidth;
uniform int textureHeight;

out vec2 textCoord;

uniform sampler2D heightTexture;

void main()
{
	gl_Position =  rMat * vec4(position, 1.0f);

	textCoord = vec2( float(position.x - 1) / ( textureWidth + 1 ) , float(1 -position.y) / ( textureHeight + 1 )   );
}
