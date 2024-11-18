#version 430


//VBO-bol erkezo valtozok
layout( location = 0 ) in vec2 vs_in_uv;


// a pipeline-ban tovább adandó értékek
out vec3 vs_out_pos;
out vec3 vs_out_norm;
out vec2 vs_out_tex;

// shader külső paraméterei - most a három transzformációs mátrixot külön-külön vesszük át
uniform mat4 world;
uniform mat4 worldIT;
uniform mat4 viewProj;

uniform float time; //pass time for waves


vec3 GetPos(float u, float v){

	return vec3(u * 20.0 - 10.0, sin(u), -v * 20.0 + 10.0);
}


void main()
{

	vec3 pos = GetPos(vs_in_uv.x, vs_in_uv.y);
	gl_Position = viewProj * world * vec4( pos, 1 );
	
	//vs_out_norm = (worldIT * vec4(vs_in_norm, 0)).xyz;

	vs_out_pos = (world * vec4(pos, 1)).xyz;
	vs_out_norm = vec3(0, 1, 0);

	vs_out_tex = vs_in_uv;
}