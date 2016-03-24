//advection fragment program

//output a color, which is the advected quantity


#version 120

uniform vec2 Resolution;
uniform vec2 playerCoords;
uniform sampler2D tex;


void main()
{
	
	vec2 fc = gl_FragCoord.xy / Resolution.xy;
	fc.y = 1 - fc.y;
	
	vec4 oldCol = texture2D( tex, gl_FragCoord.xy / Resolution.xy );
	if( abs( fc.x - playerCoords.x ) < .2 && abs( fc.y - playerCoords.y ) < .2 )
	{
		gl_FragColor = oldCol + vec4( .5, .5, .5, 1 );
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