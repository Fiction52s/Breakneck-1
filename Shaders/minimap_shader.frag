#version 120

uniform sampler2D u_texture;
uniform sampler2D u_mask;

//Rect 2
//463, 209

//Rect 3
//512, 111


void main()
{	
	//vec2 point2 = vec2( 463.0, 209.0 );
	//vec2 point3 = vec2( 512.0, 111.0 );
	vec2 fragC = gl_FragCoord.xy;
	//fragC.y = 1 - fragC.y;
	
	vec2 texturePos = vec2( gl_TexCoord[0].x / 300.0, 1.0 - gl_TexCoord[0].y / 300.0 );
	//vec2 pos = vec2( texturePos.x * 560.0, texturePos.y * 210.0 );
	//texturePos = texturePos / vec2( 12.0 );
	vec4 DiffuseColor;
	DiffuseColor = texture2D( u_texture, texturePos );//vec4( texturePos.x, texturePos.y, 0.0, 1.0 );// = texture2D(u_texture, fragC / vec2( 3.0 ) );
	DiffuseColor.a = texture2D( u_mask, texturePos ).a;
	//if( fragC.x > .5 )
	//	fragC.x = .5;
	//if( fragC.y > .5 )
	//	fragC.y = .5;
	//DiffuseColor = texture2D( u_texture, fragC / vec2( 12.0 ) );//fragC * vec2( 12.0 ) );
	//DiffuseColor.r = 1;
	gl_FragColor = DiffuseColor;//DiffuseColor;//vec4( 1.0, 0.0, 0.0, 1.0 );//DiffuseColor;
}

