#version 430

// pipeline-ból bejövő per-fragment attribútumok
in vec4 vs_out_color;
in vec4 vs_out_position;

// kimenő érték - a fragment színe
out vec4 fs_out_col;

// !!!!! VARÁZSLAT !!!!
// Erről bővebben késübb...
uniform float ElapsedTimeInSec = 0.0;
// !!!!!!!!!!!!

float sineGenerator(float x)
{
	return sin(x);
}

void main()
{
	
	//fs_out_col = vec4(1) - vs_out_color;
	//fs_out_col = out_position;
	fs_out_col = vec4(
	vec3(sineGenerator(vs_out_position.x)),
	1.0);
}
