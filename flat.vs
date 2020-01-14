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
uniform vec3 cameraPos;
uniform vec3 lightPos;

out vec3 normVec;
out vec3 camVec;
out vec3 lightVec;
out vec3 h;
out vec2 textCoord;

uniform sampler2D heightTexture;

vec2 coord(float x, float z ){
	return vec2( float(1 - x + textureOffset) / ( textureWidth ) , float(1 - z) / ( textureHeight )   );
}

float height( float x, float z ){
	vec2 coor = coord(position.x, position.z);
	vec4 heightVec = texture(heightTexture, coor);
	return heightVec.x;
}


//ind = 2*x + z
void normal(){

	vec3 norm = vec3(1.0f,1.0f,0.0f);

	if( position.x > 0 && position.x < textureWidth - 1 && position.z > 0 && position.z < textureHeight-1 ){

		//vertexes that make up the adjacent faces

		float h0 = (height( position.x - 1, position.z - 1) - height( position.x, position.z ))*heightFactor;
		float h1 = (height( position.x - 1, position.z ) - height( position.x, position.z ))*heightFactor;
		float h3 = (height( position.x, position.z - 1) - height( position.x, position.z ))*heightFactor;
		float h5 = (height( position.x, position.z + 1) - height( position.x, position.z ))*heightFactor;
		float h7 = (height( position.x + 1, position.z ) - height( position.x, position.z ))*heightFactor;
		float h8 = (height( position.x + 1, position.z + 1) - height( position.x, position.z ))*heightFactor;

		vec3 v0 = vec3( -1.0f, h0, -1.0f);
		vec3 v1 = vec3( -1.0f, h1,  0.0f);
		vec3 v3 = vec3(  0.0f, h3, -1.0f);
		vec3 v5 = vec3(  0.0f, h5,  1.0f);
		vec3 v7 = vec3(  1.0f, h7,  0.0f);
		vec3 v8 = vec3(  1.0f, h8,  1.0f);

		vec3 norm01 = cross( v0,v1 ); //face0
		vec3 norm15 = cross( v1,v5 ); //face1
		vec3 norm58 = cross( v5,v8 ); //face2
		vec3 norm87 = cross( v8,v7 ); //face3
		vec3 norm73 = cross( v7,v3 ); //face4
		vec3 norm30 = cross( v3,v0 ); //face5

		norm = normalize( norm01 + norm15 + norm58 + norm87 + norm73 + norm30 );
		//norm = vec3(h3/(heightFactor*2),0,0);

		//norm = vec3( abs(height( position.x - 1.0f, position.z - 1.0f) - height( position.x, position.z )) ,0.0f,0.0f);
	}
	else{

		float a = 0.0f;

	}

	vec4 temp = nMat*vec4(norm,0.0f) ;

	norm = normalize(vec3( temp.x, temp.y, temp.z ));

	camVec = cameraPos - position;

	temp = cMat * vec4(camVec,0.0f);

	camVec = normalize( vec3( temp.x, temp.y, temp.z ) );

	lightVec = lightPos - position;

	temp = cMat * vec4(lightVec,0.0f);

	lightVec = normalize( vec3( temp.x, temp.y, temp.z ) );

	h = normalize( camVec + lightVec );
	
	normVec = norm;

	vertexColor = vec4( abs(norm.x), abs(norm.y), abs(norm.z), 1.0f );

}

void main()
{

	textCoord = coord(position.x, position.z);

	normal();

	gl_Position =  vpMat * vec4(position.x,  height(position.x, position.z)*heightFactor , position.z, 1.0f);

}
