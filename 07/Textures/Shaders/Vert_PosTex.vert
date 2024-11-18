#version 430

// VBO-ból érkező változók
layout( location = 0 ) in vec3 vs_in_position;
layout( location = 1 ) in vec2 vs_in_tex;

in vec3 vs_in_world_pos;

// a pipeline-ban tovább adandó értékek
out vec2 vs_out_tex;
out vec4 vs_out_world_pos;

// shader külső paraméterei

// transzformációs mátrixok
uniform mat4 world;
uniform mat4 viewProj; // Egyben adjuk át, előre össze szorozva a view és projection mátrixokat.

void main()
{
	gl_Position  = viewProj * world * vec4( vs_in_position, 1.0 );
	vs_out_tex = vs_in_tex;
	vs_out_world_pos = world * vec4(vs_in_position, 1.0); //get coordinates in world
}