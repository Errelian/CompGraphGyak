#version 430

// pipeline-ból bejövő per-fragment attribútumok
in vec3 vs_out_pos;
in vec3 vs_out_norm;
in vec2 vs_out_tex;

// kimenő érték - a fragment színe
out vec4 fs_out_col;

// textúra mintavételező objektum
uniform sampler2D texImage;

uniform vec3 cameraPos;

// fenyforras tulajdonsagok
uniform vec4 lightPos = vec4( 0.0, -1.0, 0.0, 0.0); //fény iránya

uniform vec3 La = vec3(0.2, 0.2, 0.2 );
uniform vec3 Ld = vec3(1.0, 1.0, 1.0 );
uniform vec3 Ls = vec3(1.0, 1.0, 1.0 );

// uniform float lightConstantAttenuation    = 1.0;
// uniform float lightLinearAttenuation      = 0.0;
// uniform float lightQuadraticAttenuation   = 0.0;

// anyag tulajdonsagok

uniform vec3 Ka = vec3( 1.0 );
uniform vec3 Kd = vec3( 1.0 );
uniform vec3 Ks = vec3( 1.0 );

// uniform float Shininess = 1.0;

/* segítség:
	    - normalizálás: http://www.opengl.org/sdk/docs/manglsl/xhtml/normalize.xml
	    - skaláris szorzat: http://www.opengl.org/sdk/docs/manglsl/xhtml/dot.xml
	    - clamp: http://www.opengl.org/sdk/docs/manglsl/xhtml/clamp.xml
		- reflect: http://www.opengl.org/sdk/docs/manglsl/xhtml/reflect.xml
				reflect(beérkező_vektor, normálvektor);
		- pow: http://www.opengl.org/sdk/docs/manglsl/xhtml/pow.xml
				pow(alap, kitevő);
*/

void main()
{
	vec3 ambient = La * Ka;

	vec3 normal =  normalize(vs_out_norm);
	vec3 toLight = -normalize(lightPos.xyz);

	//diffuse
	float diffuseFactor = max(dot(toLight, normal), 0.0);
	vec3 diffuse = Ld * Kd * diffuseFactor;


	//specular
	vec3 reflectedRay = normalize(reflect(-toLight, vs_out_norm));
	float specularFactor = pow(max(dot(normalize(cameraPos - vs_out_pos), reflectedRay), 0.0), 16.0);
	//HF: pontfény legyen nem irany feny, guess: megnezem hogy vs_out_pos es lightPos kozott mi a vektor (vektor kivonas)

	vec3 specular = Ls * Ks * specularFactor;


	fs_out_col = vec4(ambient + diffuse + specular, 1) * texture(texImage, vs_out_tex);
}