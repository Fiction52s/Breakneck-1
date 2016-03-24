//advection fragment program

//output a color, which is the advected quantity


#version 120

uniform vec2 Resolution;
uniform vec2 playerCoords;
uniform sampler2D tex;

vec4 encode(vec2 v){
	vec4 rgba;
	
	v += 128.;

	int ix = int( v.x * 256. ); // convert to int to split accurately
	int v1x = ix / 256; // hi
	int v1y = ix - v1x * 256; // lo 

	rgba.r = float( v1x + 1 ) / 255.; // normalize
	rgba.g = float( v1y + 1 ) / 255.;

	int iy = int( v.y * 256.);
	int v2x = iy / 256; // hi
	int v2y = iy - v2x * 256; // lo 

	rgba.b = float( v2x + 1 ) / 255.;
	rgba.a = float( v2y + 1 ) / 255.;

	return rgba - 1./256.;
}

// color to velocity vector 
vec2 decode(vec4 c){
	vec2 v = vec2(0.);

	int ir = int(c.r*255.);
	int ig = int(c.g*255.);
	int irg = ir*256 + ig;
	v.x = float(irg) / 256.; 

	int ib = int(c.b*255.);
	int ia = int(c.a*255.);
	int iba = ib*256 + ia;
	v.y = float(iba) / 256.; 

	v -= 128.;
	return v;
}

void main()
{
	
	vec2 fc = gl_FragCoord.xy / Resolution.xy;
	fc.y = 1 - fc.y;
	
	vec4 oldCol = texture2D( tex, gl_FragCoord.xy / Resolution.xy );
	if( abs( fc.x - playerCoords.x ) < .05 && abs( fc.y - playerCoords.y ) < .05 )
	{
		gl_FragColor = oldCol + vec4( 1, 1, 0, 1 );//encode( decode( oldCol ) );// + vec2( 10, 5 ) );//oldCol + vec4( .5, .5, .5, 1 );//encode( vec2( -5, -5 ) );//oldCol + vec4( .5, .5, .5, 1 );
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