#version 130

uniform sampler2D u_texture;


uniform vec4 u_quad;
uniform float u_quant;

void main()
{		
	vec2 TexCoord = vec2(gl_TexCoord[0].xy);
	vec2 coord = TexCoord;
	float size = 16.0;
	ivec2 texSize = textureSize(u_texture, 0);
	vec2 tileLimit = vec2( 16, 16 * 16 ) / texSize;
	
	vec2 stuff0 = coord;//mod( coord + vec2( 0, u_quant ) * tileLimit, tileLimit ) + u_quad.xy;
	stuff0.y = mod( coord.y + u_quant, tileLimit.y) + u_quad.y;//1.0 );
	
	coord.y = mod( coord.y, 1.0 );
	
	coord.x = fract(TexCoord.x + u_quant);
	
	vec4 col = texture2D(u_texture, stuff0 );	
	gl_FragColor = vec4(col.rgb, col.a);//vec4( blendColor.rgb, col.a );
}

