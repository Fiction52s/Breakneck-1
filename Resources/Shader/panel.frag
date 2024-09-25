#version 130

uniform sampler2D u_slideTexture;
uniform vec4 baseColor;
uniform vec4 highlightColor;
uniform vec2 slideVec;
uniform vec4 slideBlendColor;
uniform vec2 slideRatio;
uniform vec2 Resolution;
uniform float breathe;

void main()
{
	
	vec2 fc = gl_FragCoord.xy;
	
	//vec2 resRatio = Resolution / vec2( 960, 540 );
	
	fc = fc / Resolution;
	//fc.y = 1.0 - fc.y;
	fc.x = 1.0 - fc.x;
	
	float distfc = sqrt( fc.x * fc.x 
		+ fc.y * fc.y );
	
	ivec2 texSize = textureSize(u_slideTexture, 0);//vec2( 400, 836 );
	vec2 slideTestSize = vec2(texSize);
	vec2 slideMult = slideTestSize / Resolution;
	
	vec2 coord;
	coord.x = mod( fc.x, slideMult.x ) / slideMult.x;
	coord.y = mod( fc.y, slideMult.y ) / slideMult.y;
	
	
	vec2 TexCoord = vec2(gl_TexCoord[0].xy);
	float dist = sqrt( TexCoord.x * TexCoord.x 
		+ TexCoord.y * TexCoord.y );
	dist = min( dist * 2.0, 1.0 );
	vec4 DiffuseColor = dist * baseColor + (1.0 - dist) * highlightColor;
	
	
	//coord.x = fc.x / Resolution.x;
	//coord.y = fract( fc.y / Resolution.y + slideQuant );
	//coord.y = mod( coord.y, slideTestSize.y );
	
	coord.x = fract( coord.x + slideVec.x );
	coord.y = fract( coord.y + slideVec.y );
	
	vec4 col = texture2D(u_slideTexture, coord );	
	
	vec4 finalColor = DiffuseColor;
	
	float bFactor = .45;
	float br = breathe;
	br = br * bFactor;
	
	br = max( 0.1, br );
	
	
	
	if( col.a != 0.0 )
	{
		br = br * dist * .5;
		br = br * col.a;
		finalColor = finalColor * (1.0 - br) + highlightColor * br;//finalColor + col;
		
		//finalColor.a = 1.0 - distfc;//baseColor + (1.0 - dist) * highlightColor;
	}
	
	finalColor.a = finalColor.a * .9;
	
	
	
	
	//gl_FragColor = vec4( DiffuseColor.rgb * col.rgb, DiffuseColor.a );
	gl_FragColor = finalColor;
	

	//vec4( blendColor.rgb, col.a );
}


