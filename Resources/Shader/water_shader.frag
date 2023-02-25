#version 130

uniform sampler2D u_texture;

uniform vec4 u_quad1;
uniform vec4 u_quad2;

uniform vec4 skyColor;
uniform vec2 topLeft;
uniform float zoom;

uniform vec2 Resolution;      //resolution of screen
uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 

uniform float u_slide;
uniform vec4 u_waterBaseColor;

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

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float brightness( vec4 col )
{
	return (0.2126*col.r + 0.7152*col.g + 0.0722*col.b);
}

void main()
{	
	float size = 128.0;
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1.0 - fc.y;
	vec2 resRatio = vec2( 960, 540 ) / Resolution;
	fc = fc * resRatio;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );	
	
	vec2 pos = topLeft + rotateUV( pixelPos, vec2( 0.0 ), u_cameraAngle );
	
	ivec2 texSize = textureSize(u_texture, 0);
	
	vec2 tileLimit = vec2( size ) / texSize;
	
	pos = mod( pos, size ) / (texSize);// * resRatio);
	//vec2 UV = mod( topLeft + pixelPos, size) / texSize;
	
	vec4 DiffuseColor;
	
	vec4 q1 = u_quad1;
	
	q1.x = q1.x + .001;
	q1.y = q1.y + .001;
	
	vec4 q2 = u_quad2;
	q2.x = q2.x + .001;
	q2.y = q2.y + .001;
	
	tileLimit.x = tileLimit.x - .001;
	tileLimit.y = tileLimit.y - .001;
	
	float scrollBase = .1;
	float extra = 0.2;
	vec2 stuff0 = mod( pos + vec2( u_slide * scrollBase * 0.5, u_slide * scrollBase ) * tileLimit, tileLimit ) + q1.xy;
	vec2 stuff1 = mod( pos + vec2( u_slide * -scrollBase * 0.5, u_slide * -scrollBase * 2.0 ) * tileLimit, tileLimit ) + q2.xy;
	vec2 stuff2 = mod( pos + vec2( u_slide * scrollBase + extra, u_slide * -scrollBase * 0.5 ) * tileLimit, tileLimit ) + q1.xy;
	vec2 stuff3 = mod( pos + vec2( u_slide * -scrollBase * 2.0 + extra, u_slide * scrollBase * 0.5 ) * tileLimit, tileLimit ) + q2.xy;
	
	vec4 colsTest[4];
	colsTest[0] = texture2D( u_texture, stuff0.xy );
	colsTest[1] = texture2D( u_texture, stuff1.xy );
	colsTest[2] = texture2D( u_texture, stuff2.xy );
	colsTest[3] = texture2D( u_texture, stuff3.xy );
	
	float test = rand( pos );
	
	float numAdded = 0;
	for( int i = 0; i < 4; ++i )
	{
		if( colsTest[i].a != 0 )
		{
			if( DiffuseColor.a == 0 )
			{
				DiffuseColor = colsTest[i];
				numAdded = numAdded + 1.0;
			}
			else
			{
				if( brightness( DiffuseColor ) < brightness( colsTest[i] ) )
				{
					DiffuseColor = colsTest[i];
				}
				//DiffuseColor = DiffuseColor + colsTest[i];
				numAdded = numAdded + 1.0;
			}
		}
	}
	
	//DiffuseColor.rgb = DiffuseColor.rgb / vec3( numAdded );
	
	test = test * 4;
	int index = int(floor( test ));
	
	//DiffuseColor = colsTest[index];
	
	/*DiffuseColor = colora0;
	if( DiffuseColor.a == 0 )
	{
		DiffuseColor = colora1;
	}
	
	if( DiffuseColor.a == 0 )
	{
		DiffuseColor = colora2;
	}
	
	if( DiffuseColor.a == 0 )
	{
		DiffuseColor = colora3;
	}*/
	
	DiffuseColor.a = DiffuseColor.a * .5;
	
	if( DiffuseColor.a == 0 )
	{
		DiffuseColor = u_waterBaseColor;
	}
	
	gl_FragColor = gl_Color * DiffuseColor;
}

