//divergence fragment program

#version 120

uniform float halfrdx; //0.5 / gridscale
uniform sampler2D w; //vector field
uniform vec2 texSize;

vec2 colorToVec( vec4 col )
{
	vec2 temp = col.xy;
	temp = (temp - .5) * 2;
	temp = normalize( temp );
	
	float mag = col.z * 256.0;
	
	temp *= vec2( mag ); //magnitude
	return temp;
}

vec3 vecToColorRGB( vec2 ve )
{
	float len = length( ve );
	vec3 temp = vec3( normalize(ve), len / 256.0 );
	temp.xy = (temp.xy * .5) + .5;
	return temp;
}

void main()
{
	vec2 wL = colorToVec(texture2D( w, (gl_FragCoord.xy - vec2( 1, 0 )) / texSize ));
	vec2 wR = colorToVec(texture2D( w, (gl_FragCoord.xy + vec2( 1, 0 )) / texSize ));
	vec2 wB = colorToVec(texture2D( w, (gl_FragCoord.xy - vec2( 0, 1 )) / texSize ));
	vec2 wT = colorToVec(texture2D( w, (gl_FragCoord.xy + vec2( 0, 1 )) / texSize ));
	
	//vec2 wL = (texture2D( w, (gl_FragCoord.xy - vec2( 1, 0 )) / texSize )).xy;
	//vec2 wR = (texture2D( w, (gl_FragCoord.xy + vec2( 1, 0 )) / texSize )).xy;
	//vec2 wB = (texture2D( w, (gl_FragCoord.xy - vec2( 0, 1 )) / texSize )).xy;
	//vec2 wT = (texture2D( w, (gl_FragCoord.xy + vec2( 0, 1 )) / texSize )).xy;
	
	
	gl_FragColor =  vec4(halfrdx * ((wR.x - wL.x) + (wT.y - wB.y )));
	//gl_FragColor.a = 1;
}