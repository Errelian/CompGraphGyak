#version 430

// pipeline-ból bejövő per-fragment attribútumok
in vec2 vs_out_tex;

// kimenő érték - a fragment színe
out vec4 fs_out_col;

//világ pos
in vec4 vs_out_world_pos;

// textúra mintavételező objektum
uniform sampler2D texImage;
uniform sampler2D texImage2;

void main()
{
	if (vs_out_world_pos.x > 0){
		fs_out_col = texture( texImage, vs_out_tex );
	}
	else
	{
		fs_out_col = texture( texImage2, vs_out_tex );
	}
}
