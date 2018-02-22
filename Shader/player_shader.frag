uniform sampler2D u_texture;

uniform bool hasPowerAirDash;
uniform bool hasPowerGravReverse;
uniform bool hasPowerBounce;
uniform bool hasPowerGrindBall;
uniform bool hasPowerTimeSlow;
uniform bool hasPowerLeftWire;
uniform bool hasPowerRightWire;

uniform float despFrame;

vec4 BallColors( vec4 DiffuseColor )
{		
	//blue
	if( DiffuseColor.rgb == vec3( 0.0, 0x66/ 255.0, 0xcc / 255.0 ) )
	{
		DiffuseColor.rgb = vec3( 0.0, 0.0, 0.0 );
	}

	//green
	//00bd60
	if( !hasPowerGravReverse && DiffuseColor.rgb == vec3( 0.0, 0xbd / 255.0, 0x60 / 255.0 ) )
	{
		DiffuseColor.rgb = vec3( 0.0, 0.0, 0.0 );
	}
	
	//yellow
	if( !hasPowerBounce && DiffuseColor.rgb == vec3( 0xff / 255.0, 0xf0 / 255.0, 0.0 ) )
	{
		DiffuseColor.rgb = vec3( 0.0, 0.0, 0.0 );
	}
	
	//orange
	if( !hasPowerGrindBall && DiffuseColor.rgb == vec3( 0xff / 255.0, 0xbb / 255.0, 0.0 ) )
	{
		DiffuseColor.rgb = vec3( 0.0, 0.0, 0.0 );
	}
	
	//red
	if( !hasPowerTimeSlow && DiffuseColor.rgb == vec3( 0xff / 255.0, 0x22 / 255.0, 0.0 ) )
	{
		DiffuseColor.rgb = vec3( 0.0, 0.0, 0.0 );
	}
	
	//magenta
	if( (!hasPowerRightWire && !hasPowerLeftWire) && DiffuseColor.rgb == vec3( 0xff / 255.0, 0.0, 0xff / 255.0 ) )
	{
		DiffuseColor.rgb = vec3( 0.0, 0.0, 0.0 );
	}
	return DiffuseColor;
}

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(u_texture, gl_TexCoord[0].xy);
	pixel = BallColors( pixel );
    // multiply it by the color
    gl_FragColor = gl_Color * pixel;
}