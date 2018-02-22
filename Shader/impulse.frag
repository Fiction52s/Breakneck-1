//advection fragment program

//output a color, which is the advected quantity


#version 120

uniform vec2 Resolution;
uniform vec2 playerCoords;
uniform sampler2D tex;

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
	
	vec2 fc = gl_FragCoord.xy / Resolution.xy;
	fc.y = 1 - fc.y;
	
	vec4 oldCol = texture2D( tex, gl_FragCoord.xy / Resolution.xy );
	if( abs( fc.x - playerCoords.x ) < .05 && abs( fc.y - playerCoords.y ) < .05 )
	{
		gl_FragColor = oldCol + vec4( 1, 1, .1, 1 );//vec4( vecToColorRGB( vec2( -.0001, -.0005 ) ), 1 );//vec4( 1, 1, .1, 1 );//+ vec4( vecToColorXY( vec2( -1, 0 ) ), 0, 0 );
	}
	else
	{
		gl_FragColor = oldCol; //gl_Color;//vec4( 0, 0, 0, 0 );//gl_Color;
	}
	
	//vec2 vel = texture2D(u, gl_FragCoord.xy ).xy;
	
	//follow the vector field back in time
	//vec2 pos = gl_FragCoord.xy - vec2(timestep) * vec2(rdx) * ;
	//gl_FragColor = f4texRECTbilerp( x, pos );//textureBicubic( x, pos );
}