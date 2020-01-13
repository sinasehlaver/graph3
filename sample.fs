#version 330 core
out vec4 color;
in vec4 vertexColor;

in vec2 textCoord;

uniform sampler2D colorTexture;

void main()
{
	color = texture(colorTexture , textCoord );
}
