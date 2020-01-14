#version 330 core
out vec4 color;

out vec3 lightIntensity;

in vec4 vertexColor;

in vec2 textCoord;

uniform sampler2D colorTexture;

vec4 ambRef;
vec4 ambLight;
vec4 specRef;
vec4 specLight;
int specExp;
vec4 difRef;
vec4 difLight;

out vec4 normVec;

void main()
{
	lightIntensity = vec3( 1.0f,1.0f,1.0f );

	ambRef = vec4(0.25f,0.25f,0.25f,1.0f);
	ambLight = vec4(0.3f,0.3f,0.3f,1.0f);
	specRef = vec4(1.0f,1.0f,1.0f,1.0f);
	specLight = vec4(1.0f,1.0f,1.0f,1.0f);
	specExp = 100;
	difRef = vec4(1.0f,1.0f,1.0f,1.0f);
	difLight = vec4(1.0f,1.0f,1.0f,1.0f);

	vec4 textureColor = texture(colorTexture , textCoord );

	color = textureColor;
}
