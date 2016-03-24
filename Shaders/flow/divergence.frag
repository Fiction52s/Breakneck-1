//divergence fragment program

#version 120

uniform float halfrdx; //0.5 / gridscale
uniform sampler2D w; //vector field
uniform vec2 texSize;

void main()
{
	vec4 wL = texture2D( w, (gl_FragCoord.xy - vec2( 1, 0 )) / texSize );
	vec4 wR = texture2D( w, (gl_FragCoord.xy + vec2( 1, 0 )) / texSize );
	vec4 wB = texture2D( w, (gl_FragCoord.xy - vec2( 0, 1 )) / texSize );
	vec4 wT = texture2D( w, (gl_FragCoord.xy + vec2( 0, 1 )) / texSize );
	
	gl_FragColor.rgb = vec3 (halfrdx * ((wR.x - wL.x) + (wT.y - wB.y) ) );
	gl_FragColor.a = 1;
}