#version 330 core
layout ( location = 0 ) in vec4 position;
uniform mat4 cMat;
uniform mat4 vpMat;
uniform mat4 pMat;
uniform mat4 nMat;
out vec4 vertexColor;

uniform int textureWidth;
uniform int textureHeight;

uniform int textureOffset;
uniform float heightFactor;
uniform vec3 cameraPos;
uniform vec3 lightPos;

out vec3 normVec;
out vec3 camVec;
out vec3 lightVec;
out vec3 h;
out vec2 textCoord;

float radius = 350.0f;

uniform sampler2D heightTexture;

float height(vec2 uv){
	vec4 heightVec = texture(heightTexture, uv);
	return heightVec.x;
}


vec3 posCalc( float alpha, float beta ){
	float x = radius*sin(beta)*cos(alpha);
	float y = radius*sin(beta)*sin(alpha);
	float z = radius*cos(beta);

	return vec3(x,y,z);
}

//ind = 2*x + z

void main()
{

	textCoord = vec2(position.z, position.w);

	vec3 reelPos = posCalc( position.x, position.y );

	gl_Position =  vpMat * vec4( reelPos, 1.0f);

}
