#version 120

uniform sampler2D u_texture;
uniform float level;
uniform float quant;

//Rect 2
//463, 209

//Rect 3
//512, 111


void main()
{	
	vec2 point2 = vec2( 463, 209 );
	vec2 point3 = vec2( 512, 111 );
	//vec2 fragC = gl_FragCoord.xy;
	//fragC.y = 1 - fragC.y;
	
	vec2 texturePos = vec2( gl_TexCoord[0].x, gl_TexCoord[0].y );
	vec2 pos = vec2( texturePos.x * 560, texturePos.y * 210 );
	
	vec4 DiffuseColor = texture2D(u_texture, texturePos );
	if( DiffuseColor.a > 0 )
	{
		if( level == 0 )
		{
			if( DiffuseColor.rgb == vec3( 117/ 255.0, 193/ 255.0, 255/ 255.0 ) )
			{
				DiffuseColor.a = float(pos.x <= quant * 463);
			}
			else
			{
				DiffuseColor.a = 0;
			}
			
			
		}
		else if( level == 1 )
		{
			//if( color mid )
			vec2 along = normalize( point3 - point2 );
			float len = length( point3 - point2 );
			float q = dot( pos - point2, along );
			
			if( DiffuseColor.rgb == vec3( 115/ 255.0, 133/ 255.0, 255/ 255.0 ) )
			{
				DiffuseColor.a = float(q <= quant * len);
			}
			else if( DiffuseColor.rgb == vec3( 113 / 255.0, 66/ 255.0, 255/ 255.0 ) )
			{
				DiffuseColor.a = 0;
			}
		}
		else if( level == 2 )
		{
			float height = 111;
			if( DiffuseColor.rgb == vec3( 113 / 255.0, 66/ 255.0, 255/ 255.0 ) )
			{
				DiffuseColor.a = float((height - pos.y) < quant * height);
			}
		}
	}
	gl_FragColor = DiffuseColor;
	
}

