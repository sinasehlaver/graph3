#version 330 core
layout ( location = 0 ) in vec3 position;
uniform mat4 cMat;
uniform mat4 vpMat;
uniform mat4 pMat;
uniform mat4 nMat;
out vec4 vertexColor;

uniform int textureWidth;
uniform int textureHeight;
uniform float heightFactor;

out vec2 textCoord;

uniform sampler2D heightTexture;

void main()
{

	textCoord = vec2( float(1- position.x) / ( textureWidth + 1 ) , float(1 -position.z) / ( textureHeight + 1 )   );

	vec4 heightVec = texture(heightTexture, textCoord);

	gl_Position =  vpMat * vec4(position.x,  heightVec.x*heightFactor, position.z, 1.0f);

}
