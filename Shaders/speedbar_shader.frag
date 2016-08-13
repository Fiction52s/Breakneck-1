#version 120

uniform sampler2D u_texture;
uniform float onPortion; 

void main()
{	
	vec4 DiffuseColor = texture2D( u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y ) );
	vec2 fc = vec2( 1920 - gl_FragCoord.x, 1080 - gl_FragCoord.y );
	float rp = 1.0 - onPortion;
	float trueBot = 288.0 - 16.0;
	float currTop = 21;
	float currBot = currTop + rp * ( trueBot - currTop );
	DiffuseColor.a = 1.0 * float( fc.y > currBot ) * DiffuseColor.a;
	
	gl_FragColor = DiffuseColor;//vec4( 1.0, 0.0, 0.0, 1.0 );//DiffuseColor;
	
}

