#version 120

//our texture samplers
uniform sampler2D u_texture;   //diffuse map

uniform float zoom;
uniform vec2 Resolution;
uniform vec2 pos;
uniform vec2 lightpos;
uniform float newscreen;
uniform vec2 topLeft;

uniform vec2 bubble0;
uniform float b0Frame;
uniform vec2 bubble1;
uniform float b1Frame;
uniform vec2 bubble2;
uniform float b2Frame;
uniform vec2 bubble3;
uniform float b3Frame;
uniform vec2 bubble4;
uniform float b4Frame;
uniform vec2 bubble5;
uniform float b5Frame;

uniform float bubbleRadius;

const int numBubbles = 6;
struct Bubble
{
	bool on;
	vec2 pos;
	float frame;
};
Bubble bubbles[numBubbles];


void InitBubbles()
{
	if( b0Frame == 0 )
	{
		bubbles[0].on = false;
	}
	else
	{
		bubbles[0].on = true;
		bubbles[0].pos = bubble0;
		bubbles[0].frame = b0Frame;
	}
	
	if( b1Frame == 0 )
	{
		bubbles[1].on = false;
	}
	else
	{
		bubbles[1].on = true;
		bubbles[1].pos = bubble1;
		bubbles[1].frame = b1Frame;
	}
	
	if( b2Frame == 0 )
	{
		bubbles[2].on = false;
	}
	else
	{
		bubbles[2].on = true;
		bubbles[2].pos = bubble2;
		bubbles[2].frame = b2Frame;
	}
	
	if( b3Frame == 0 )
	{
		bubbles[3].on = false;
	}
	else
	{
		bubbles[3].on = true;
		bubbles[3].pos = bubble3;
		bubbles[3].frame = b3Frame;
	}
	
	if( b4Frame == 0 )
	{
		bubbles[4].on = false;
	}
	else
	{
		bubbles[4].on = true;
		bubbles[4].pos = bubble4;
		bubbles[4].frame = b4Frame;
	}
	
	if( b5Frame == 0 )
	{
		bubbles[5].on = false;
	}
	else
	{
		bubbles[5].on = true;
		bubbles[5].pos = bubble5;
		bubbles[5].frame = b5Frame;
	}
}

void main()
{
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1 - fc.y;
	fc = fc * vec2( 1920, 1080 ) / Resolution;//vec2( 1920, 1080 ) / Resolution;//vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
	
	vec2 fragC = gl_FragCoord.xy;
	fragC.y = 1 - fragC.y;
	
	//vec2 pos = pixelPos + topLeft;
	float xtex = gl_TexCoord[0].x;
	float ytex = gl_TexCoord[0].y;
	
	vec4 sum = vec4( 0.0 );
	float blur = 1.0;
	//vec2 tc = gl_TexCoord[0].xy; 
	vec2 tc = gl_FragCoord.xy;//gl_TexCoord[0].xy; 
	
	float hstep = 1;
	float vstep = 0;
	
	sum += texture2D(u_texture, vec2(tc.x - 4.0*blur*hstep, tc.y - 4.0*blur*vstep)) * 0.0162162162;
    sum += texture2D(u_texture, vec2(tc.x - 3.0*blur*hstep, tc.y - 3.0*blur*vstep)) * 0.0540540541;
    sum += texture2D(u_texture, vec2(tc.x - 2.0*blur*hstep, tc.y - 2.0*blur*vstep)) * 0.1216216216;
    sum += texture2D(u_texture, vec2(tc.x - 1.0*blur*hstep, tc.y - 1.0*blur*vstep)) * 0.1945945946;

    sum += texture2D(u_texture, vec2(tc.x, tc.y)) * 0.2270270270;

    sum += texture2D(u_texture, vec2(tc.x + 1.0*blur*hstep, tc.y + 1.0*blur*vstep)) * 0.1945945946;
    sum += texture2D(u_texture, vec2(tc.x + 2.0*blur*hstep, tc.y + 2.0*blur*vstep)) * 0.1216216216;
    sum += texture2D(u_texture, vec2(tc.x + 3.0*blur*hstep, tc.y + 3.0*blur*vstep)) * 0.0540540541;
    sum += texture2D(u_texture, vec2(tc.x + 4.0*blur*hstep, tc.y + 4.0*blur*vstep)) * 0.0162162162;

    //discard alpha for our simple demo, multiply by vertex color and return
    
	
	
	float diffx = ( xtex - .5 );
	float diffy = abs( ytex - .5 );
	
	float blurFactor = .01;
	vec4 DiffuseColor = texture2D(u_texture, vec2( xtex, ytex ) );
	
	InitBubbles();
	
	vec4 col = DiffuseColor;
	
	
	
	for( int i = 0; i < numBubbles; i++ )
	{
		if( bubbles[i].on )
		{
			float D = length( bubbles[i].pos * Resolution.xy - vec2( fragC.x, fragC.y) ) * zoom;
			vec2 dir = bubbles[i].pos * Resolution.xy - vec2( fragC.x, fragC.y) * zoom;
			dir = normalize( dir );
			float trueRad = bubbleRadius * 2;
			float expandFrames = 20;
			float trueFrame = ( 240 - bubbles[i].frame );
			if( trueFrame < expandFrames )
			{
				trueRad = (trueRad + 80 ) * ( 240 - bubbles[i].frame ) / expandFrames;
			}
			else if( trueFrame == expandFrames )
			{
				trueRad = (trueRad + 40 ) * ( 240 - bubbles[i].frame - 1 ) / expandFrames;
			}
			
			
			if( D <= trueRad )
			{
				/*if( bubbles[i].frame > 240 - expandFrames )
				{
					
					col = texture2D( u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y ) );
				}*/
				
				if( trueFrame <= expandFrames )
				{
					col.r = 1 - col.r;
					col.g = 1 - col.g;
					col.b = 1 - col.b;
				}
				else
				{
					float gray = dot(col.rgb, vec3(0.299, 0.587, 0.114));
					col.r = gray + .5;
					col.g = gray;
					col.b = gray;
				}
				break;
			}
		}
	}
	
	/*if( pixelPos.y < 540 * newscreen )
	{
		vec4 DiffuseColor = texture2D( u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y - (1 - newscreen) ) );
		gl_FragColor = col * DiffuseColor * vec4( 1, 0, 0, .5 );
	}
	else
	{
		vec4 DiffuseColor = texture2D(u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y + newscreen) );
		gl_FragColor = col * DiffuseColor;
	}*/
	
	
	gl_FragColor = col;// * vec4(sum.rgb, 1.0);
}

