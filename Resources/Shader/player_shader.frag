#version 130
uniform sampler2D u_texture;
uniform sampler2D u_auraTex;
uniform sampler2D u_auraTex2;
uniform vec4 u_auraColor;
uniform vec4 u_quad;
uniform float u_slide;
uniform float u_invincible;
uniform float u_super;
uniform vec4 u_palette[164];

bool found;
vec2 texelSize;
vec4 pixel2;
float len;
vec2 pos;

void main()
{
	int dist = 2;
    vec4 pixel = texture2D(u_texture, gl_TexCoord[0].xy);
	
	//these 2 lines do the palette stuff
	int paletteTexIndex = int(pixel.r * 255.0);
	pixel = vec4( u_palette[paletteTexIndex].rgb, pixel.a);
	
	ivec2 size = textureSize(u_texture, 0);
	texelSize.x = float(size.x);
	texelSize.y = float(size.y);
	texelSize.x = 1.0 / texelSize.x;
	texelSize.y = 1.0 / texelSize.y;
	vec2 distVec = vec2( dist, dist );
	float distLen = length( distVec );
	
	vec2 six = vec2( 64.0 );
	vec2 p = modf( gl_FragCoord.xy, six ) / six;
	
	vec2 quadSize = u_quad.zw - u_quad.xy;
	if( pixel.a == 0.0 )
	{
		found = false;
		for( int x = -dist; x <= dist && !found; ++x )
		{
			for( int y = -dist; y <= dist && !found; ++y )
			{
				pos = vec2( x, y );
				len = length( pos ) / distLen;
				pixel2 = texture2D( u_texture, vec2( 
				clamp( gl_TexCoord[0].x + texelSize.x * pos.x, u_quad.x + .0001, u_quad.z - .0001 ), 
				clamp( gl_TexCoord[0].y + texelSize.y * pos.y, u_quad.y + .0001, u_quad.w - .0001 ) ) );
				
				if( pixel2.a != 0.0 )
				{
					//vec2 blah = vec2( 1.0, 1.0 );
					//vec2 stuff = ((gl_TexCoord[0].xy - u_quad.xy) / quadSize.xy) + vec2( 0.0, u_slide * .7);//, 0.0 );
					//vec2 stuff2 = ((gl_TexCoord[0].xy - u_quad.xy) / quadSize.xy) + vec2( 0.0, u_slide * 1.3);//, 0.0 );
					//vec4 colora0 = texture2D( u_auraTex, stuff.xy );
					//vec4 colora1 = texture2D( u_auraTex2, stuff2.xy );
					
					pixel = u_auraColor;
					found = true;
				}
			}
		}
	}
	else
	{
		if( u_super == 1.0 )
		{
			pixel.rgb = vec3(1.0, 0.0, 0.0 );
		}
		else if( u_super == 2.0 )
		{
			pixel.rgb = vec3( 1.0, 0.0, 1.0 );
		}
		
		pixel.rgb = pixel.rgb + vec3( 1.0 ) * u_invincible * .5;
		
	}
	
	
    gl_FragColor = gl_Color * pixel;
}