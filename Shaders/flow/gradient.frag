//gradient fragment program

#version 120

uniform float halfrdx; //0.5 / gridscale
uniform sampler2D w; //velocity
uniform sampler2D p; //pressure

void main()
{
	float pL = texture2D( p, gl_FragCoord.xy - vec2( 1, 0 ) ).x;
	float pR = texture2D( p, gl_FragCoord.xy + vec2( 1, 0 ) ).x;
	float pB = texture2D( p, gl_FragCoord.xy - vec2( 0, 1 ) ).x;
	float pT = texture2D( p, gl_FragCoord.xy + vec2( 0, 1 ) ).x;
	
	vec4 col = texture2D( w, gl_FragCoord.xy );
	col.xy -= vec2(halfrdx) * vec2( pR - pL, pT - pB );
	
	gl_FragColor = col;
}