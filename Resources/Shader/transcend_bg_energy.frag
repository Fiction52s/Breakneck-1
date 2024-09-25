#version 130

uniform sampler2D u_texture;
uniform sampler2D u_scrollTexture;
uniform float quant;
uniform float zoom;
uniform float u_depth;

uniform vec2 topLeft;
uniform float u_cameraAngle;

vec2 rotateUV(in vec2 uv, in vec2 pivot, in float rotation) {
    float sine = sin(rotation);
    float cosine = cos(rotation);

    uv -= pivot;
	float tempX = uv.x * cosine - uv.y * sine;
    uv.y = uv.x * sine + uv.y * cosine;
	uv.x = tempX;
    uv += pivot;

    return uv;
}

void main()
{		
	vec2 TexCoord = vec2(gl_TexCoord[0].xy);
	vec2 coord = TexCoord;
	
	//coord.x = fract(TexCoord.x + quant);
	
	vec4 col = texture2D(u_texture, coord );
	
	if( col.r > .9 && col.b > .9 && col.g < 0.1 )
	{
		vec2 fc = gl_FragCoord.xy;
		fc.y = 1.0 - fc.y;
		vec2 resRatio = vec2(2.0);//vec2( 960, 540 ) / Resolution;
		fc = fc * resRatio;
		float z = 1.0;
		vec2 pixelPos = vec2( fc.x * z, fc.y * z );
		
		ivec2 scrollTexSize = textureSize(u_scrollTexture, 0);
		vec2 tl = topLeft;
		tl.x = tl.x * u_depth * resRatio.x;
		
		vec2 pos = tl + rotateUV( pixelPos, vec2( 0.0 ), 
			u_cameraAngle );
		
		vec2 slideCoord = pos / scrollTexSize;
		slideCoord.x = fract( slideCoord.x);
		slideCoord.y = fract( slideCoord.y);
		
		slideCoord.y = fract(slideCoord.y + quant);
		vec4 col2 = texture2D( u_scrollTexture, slideCoord );
		col = col2;
	}
	
	gl_FragColor = col;//vec4(col.rgb, col.a);//vec4( blendColor.rgb, col.a );
}

