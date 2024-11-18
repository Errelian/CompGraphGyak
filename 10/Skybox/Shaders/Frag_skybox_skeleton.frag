#version 430

// pipeline-ból bejövő per-fragment attribútumok
in vec3 vs_out_pos;

out vec4 fs_out_col;

uniform vec3 groundColor = vec3(0.2, 0.2, 0.2);
uniform vec3 skyColor = vec3(0.2, 0.2, 0.5);

// skybox textúra
uniform samplerCube skyboxTexture;

void main()
{
	//fs_out_col = vec4( vs_out_pos.y,vs_out_pos.y,vs_out_pos.y,1.0);

	//fs_out_col = vec4( mix(groundColor, skyColor, normalize(vs_out_pos).y *0.5 + 0.5), 1.0);

	fs_out_col = vec4(texture(skyboxTexture, normalize(vs_out_pos)).xyz, 1 );
}