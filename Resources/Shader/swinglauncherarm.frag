#version 130

uniform sampler2D u_texture;

uniform vec4 u_quad;
uniform float u_quant;

void main()
{	
	float size = 128.0;
	vec2 TexCoord = vec2(gl_TexCoord[0].xy);
	vec2 coord = TexCoord;
	
	ivec2 texSize = textureSize(u_texture, 0);
	vec2 tileLimit = vec2( size, size ) / texSize;
	
	vec4 q = u_quad;
	
	//float extra = .1;
	
	//q.x = q.x + extra;
	//q.y = q.y + extra;
	
	//tileLimit.x = tileLimit.x - extra;
	//tileLimit.y = tileLimit.y - extra;
	
	vec2 stuff0 = coord;
	
	//stuff0.x = q.x + 
	stuff0.y = mod( coord.y + u_quant, tileLimit.y) + q.y;
	
	vec4 col = texture2D(u_texture, stuff0 );	
	gl_FragColor = vec4(col.rgb, col.a);
}

