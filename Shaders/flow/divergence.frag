//divergence fragment program

#version 120

uniform float halfrdx; //0.5 / gridscale
uniform sampler2D w; //vector field
uniform vec2 texSize;

vec2 colorToVec( vec4 col )
{
	vec2 temp = col.xy;
	temp = (temp - .5) * 2;
	float mag = col.z * 256.0;
	temp *= vec2( mag ); //magnitude
	return temp;
}

vec3 vecToColorRGB( vec2 ve )
{
	float len = length( ve );
	vec3 temp = vec3( normalize(ve), len / 256.0 );
	temp = (temp * .5) + .5;
	return temp;
}

void main()
{
	vec2 wL = colorToVec(texture2D( w, (gl_FragCoord.xy - vec2( 1, 0 )) / texSize ));
	vec2 wR = colorToVec(texture2D( w, (gl_FragCoord.xy + vec2( 1, 0 )) / texSize ));
	vec2 wB = colorToVec(texture2D( w, (gl_FragCoord.xy - vec2( 0, 1 )) / texSize ));
	vec2 wT = colorToVec(texture2D( w, (gl_FragCoord.xy + vec2( 0, 1 )) / texSize ));
	
	
	gl_FragColor =  vec4(halfrdx * (wR.x - wL.x) + (wT.y - wB.y ));//vec3 (halfrdx * ((wR0.x - wL0.x) + (wT0.y - wB0.y) ) ));
	//gl_FragColor.a = 1;
}