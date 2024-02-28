#version 430

// pipeline-ból bejövő per-fragment attribútumok
in vec4 vs_out_color;
in vec4 vs_out_pos;

// kimenő érték - a fragment színe
out vec4 fs_out_col;

// !!!!! VARÁZSLAT !!!!
// Erről bővebben késübb...
uniform float ElapsedTimeInSec = 0.0;
// !!!!!!!!!!!!

const float PI = 3.14;

float myFunc(float t)
{
	return cos(t * 2.0 * PI + ElapsedTimeInSec);
}

vec2 cplx_mul(vec2 u, vec2 v)
{
	return vec2(
		u.x * v.x - u.y * v.y,
		u.x * v.y + u.y * v.x
	);
}

void main()
{
	//fs_out_col = vec4(vec3(myFunc(vs_out_pos.x)),1.0);

	vec2 c = vs_out_pos.xy;
	vec2 z = c;
	for (int i =0; i< 30; i++){
		z = cplx_mul(z, z) + c;
	}

	if(length(z) < 2){
		fs_out_col = vs_out_color;	
	}
	else{
		discard;
	}

}
