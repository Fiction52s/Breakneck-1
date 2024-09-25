#version 130

uniform sampler2D u_texture;
uniform float quant;

uniform float breakPosQuant;
uniform float breakQuant;
uniform float numReps;
uniform float tile;

void main()
{
	float numTiles = 14.0;

	vec2 TexCoord = vec2(gl_TexCoord[0].xy);
	vec2 coord = TexCoord;
	coord.y = fract( TexCoord.y + quant * 2.0 );
	coord.x = coord.x / numTiles + ( 1 / numTiles ) * tile;
	vec4 col = texture2D(u_texture, coord );
	
	
	float downDiff = 1.0 - breakPosQuant;
	float upDiff = breakPosQuant;
	
	float down = min( breakPosQuant + breakQuant * downDiff, 1.0 );
	float up = max( breakPosQuant - breakQuant * upDiff, 0.0 );
	float trueY = fract(TexCoord.y / numReps);
	if( breakQuant > 0.0 )
	{
		col.a *= float( (trueY < up || trueY > down ) );
	}
	
	
	gl_FragColor = col;
}
