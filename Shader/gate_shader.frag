uniform sampler2D u_texture;
uniform float quant;
uniform float tile;

uniform float fadeQuant;

void main()
{
	vec2 TexCoord = vec2(gl_TexCoord[0].xy);
	vec2 coord = TexCoord;
	
	float doubleStripWidth = 1.0 / 8.0;
	float stripWidth = doubleStripWidth / 2.0;
	float checkScrollDown = mod( coord.x, doubleStripWidth ); 
	float checkScrollUp = mod( 1.0 - coord.x, doubleStripWidth );
	coord.y = fract( TexCoord.y + quant * step( stripWidth , checkScrollDown ) - quant * step( stripWidth, checkScrollUp  ) );
	coord.x = coord.x / 8.0 + (1.0/8.0) * tile;
	vec4 col = texture2D(u_texture, coord );
	
	col.a *= 1.0 - fadeQuant;
	
	gl_FragColor = col;
}

