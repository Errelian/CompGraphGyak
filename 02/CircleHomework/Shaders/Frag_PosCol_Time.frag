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

uniform vec2 viewPortResolution;

//immutables for calculations
const vec2 center = vec2(0.5, 0.5); //I don't know why, but (0.0, 0.0) was in the lower left corner
const float step = 0.0005;

void main()
{
	float radius = abs(sin(ElapsedTimeInSec) / 2);

	vec2 currentFrag = gl_FragCoord.xy / viewPortResolution;

	float distanceFromCenter = distance(currentFrag, center);

	bool notCircleEdge = abs(radius - distanceFromCenter) > step; //to avoid issues stemming from floating point imprecision

	if (notCircleEdge)
	{
		discard;
	}
	fs_out_col = vec4(0, 0, 0, 1);
}
