//gradient fragment program

#version 120

uniform float halfrdx; //0.5 / gridscale
uniform sampler2D w; //velocity
uniform sampler2D p; //pressure
uniform vec2 texSize;

vec2 colorToVec( vec4 col )
{
	vec2 temp = col.xy;
	temp = (temp - .5) * 2;
	return temp;
}

vec4 vecToColor( vec2 ve )
{
	vec4 temp = vec4( ve, 0, 1 );
	temp.xy = (temp.xy * .5) + .5;
	return temp;
}

void main()
{
	float pL = ( texture2D( p, (gl_FragCoord.xy - vec2( 1, 0 )) / texSize ).x );
	float pR = ( texture2D( p, (gl_FragCoord.xy + vec2( 1, 0 )) / texSize ).x );
	float pB = ( texture2D( p, (gl_FragCoord.xy - vec2( 0, 1 )) / texSize ).x );
	float pT = ( texture2D( p, (gl_FragCoord.xy + vec2( 0, 1 )) / texSize ).x );
	
	vec4 col = texture2D( w, gl_FragCoord.xy / texSize );
	vec2 colVec = colorToVec( col ) * 256 / 60.0;
	
	vec2 grad = vec2(halfrdx) * vec2( pR - pL, pT - pB );
	
	col.xy -= grad;
	//col.xy /= 256 * 60.0;
	//colVec -= grad;
	//colVec.xy *= 256 / 60.0;
	//col = vecToColor( colVec );
	
	gl_FragColor = col;
}