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

in vec3 normVec;
in vec3 h;
in vec3 lightVec;


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


	float alpha = clamp( dot( normVec, h ), 0, 1 );//Specular
	float theta = clamp( dot( normVec, lightVec ), 0, 1 );//Diffuse

	vec4 amb = ambRef * ambLight;
	vec4 dif = difRef * difLight;
	vec4 spec = specRef*pow(alpha,specExp)*specLight;

	vec4 comb = amb + dif + spec;

	vec3 comVec = vec3(comb.x,comb.y,comb.z);
	vec3 textColor = vec3( textureColor.x, textureColor.y, textureColor.z ) ;


	color = vec4(clamp( comVec * textColor, 0, 1 ),1.0f);//textureColor;
}
