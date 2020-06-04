#version 130
uniform sampler2D u_texture;
uniform sampler2D u_auraTex;
uniform sampler2D u_auraTex2;
uniform vec4 u_auraColor;
uniform vec4 u_quad;
uniform float u_slide;

bool found;
vec2 texelSize;
vec4 pixel2;
float len;
vec2 pos;

void main()
{
	int dist = 2;
    vec4 pixel = texture2D(u_texture, gl_TexCoord[0].xy);
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
				clamp( gl_TexCoord[0].x + texelSize.x * pos.x, u_quad.x, u_quad.z ), 
				clamp( gl_TexCoord[0].y + texelSize.y * pos.y, u_quad.y, u_quad.w ) ) );
				
				if( pixel2.a != 0.0 )
				{
					vec2 blah = vec2( 1.0, 1.0 );
					vec2 stuff = ((gl_TexCoord[0].xy - u_quad.xy) / quadSize.xy) + vec2( 0.0, u_slide * .7);//, 0.0 );
					vec2 stuff2 = ((gl_TexCoord[0].xy - u_quad.xy) / quadSize.xy) + vec2( 0.0, u_slide * 1.3);//, 0.0 );
					vec4 colora0 = texture2D( u_auraTex, stuff.xy );
					vec4 colora1 = texture2D( u_auraTex2, stuff2.xy );
					pixel = colora0;//colora0 * colora1;
					if( colora1.a != 0 )
					pixel = colora0 * colora1;//colora0.a;

					//pixel.a = colora0.a + colora1.a;
					//pixel.a = max( colora0.a, colora1.a );
					//pixel.a = .8;
					//pixel = u_auraColor;
					found = true;
				}
			}
		}
	}
	
    gl_FragColor = gl_Color * pixel;
}