#version 330 core
layout ( location = 0 ) in vec3 position;
uniform mat4 cMat;
uniform mat4 vpMat;
uniform mat4 pMat;
uniform mat4 nMat;
out vec4 vertexColor;

uniform int textureWidth;
uniform int textureHeight;
uniform int textureOffset;
uniform float heightFactor;

out vec4 normVec;

out vec2 textCoord;

uniform sampler2D heightTexture;

vec2 coord(float x, float z ){
	return vec2( float(1 - x + textureOffset) / ( textureWidth + 1 ) , float(1 - z) / ( textureHeight + 1 )   );
}

float height( float x, float z ){
	vec2 coor = coord(position.x, position.z);
	vec4 heightVec = texture(heightTexture, coor);
	return heightVec.x*heightFactor;
}

void main()
{

	textCoord = coord(position.x, position.z);

	gl_Position =  vpMat * vec4(position.x,  height(position.x, position.z) , position.z, 1.0f);

}
