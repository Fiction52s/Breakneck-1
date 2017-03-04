#version 120

uniform sampler2D u_texture;
//uniform sampler2D u_mask;
uniform vec2 imageSize;

//Rect 2
//463, 209

//Rect 3
//512, 111


void main()
{	
	vec2 center = vec2( imageSize.x / 2, imageSize.y / 2 );
	vec2 fragC = gl_FragCoord.xy; // imageSize;
	//fragC.y = -fragC.y;
	
	//300, 300
	vec2 texturePos = vec2( gl_TexCoord[0].x, (1.0 - gl_TexCoord[0].y) );
	
	vec2 tpos = texturePos * imageSize;
	
	//vec4 DiffuseColor = gl_Color;
	
	vec4 DiffuseColor = texture2D( u_texture, texturePos );

	float len = length( tpos - center );
	
	float startFade = 150.0;
	float diff = 250.0 - startFade;
	float base = .8;
	
	float end = 1.0 - ( len - startFade )/ diff;
	end = end * base;
	
	distFact = distFact - realFact;
	if( len > startFade )// && len < 250.0 )
		DiffuseColor.a = end;// * float( len < 250.0 );//1.0 - .2;//length( fragC - vec2(.5) ); //length( fragC //texture2D( u_mask, texturePos ).a;
	else
		DiffuseColor.a = base;
		

	gl_FragColor = DiffuseColor;
}

