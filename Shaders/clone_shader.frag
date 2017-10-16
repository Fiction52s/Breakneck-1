//#version 120

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
//uniform vec2 bubble5;
//uniform float b5Frame;

//uniform float bubbleRadius;
uniform float bubbleRadius0;
uniform float bubbleRadius1;
uniform float bubbleRadius2;
uniform float bubbleRadius3;
uniform float bubbleRadius4;

const int numBubbles = 5;
const int maxPlayers = 4;
uniform float totalBubbles;
uniform float numPlayers;
uniform float bubbleRadius[20];
uniform float bFrame[20];
uniform vec2 bPos[20];

struct Bubble
{
	bool on;
	vec2 pos;
	float frame;
	float radius;
};
Bubble bubbles[numBubbles];


//super super inefficient
void InitBubbles()
{
	if( b0Frame == 0 )
	{
		bubbles[0].on = false;
	}
	else
	{
		bubbles[0].on = true;
		bubbles[0].pos = bPos[0];
		bubbles[0].frame = bFrame[0];
		bubbles[0].radius = bubbleRadius[0];
	}
	
	if( b1Frame == 0 )
	{
		bubbles[1].on = false;
	}
	else
	{
		bubbles[1].on = true;
		bubbles[1].pos = bPos[1];
		bubbles[1].frame = bFrame[1];
		bubbles[1].radius = bubbleRadius[1];
	}
	
	if( b2Frame == 0 )
	{
		bubbles[2].on = false;
	}
	else
	{
		bubbles[2].on = true;
		bubbles[2].pos = bPos[2];
		bubbles[2].frame = bFrame[2];
		bubbles[2].radius = bubbleRadius[2];
	}
	
	if( b3Frame == 0 )
	{
		bubbles[3].on = false;
	}
	else
	{
		bubbles[3].on = true;
		bubbles[3].pos = bPos[3];
		bubbles[3].frame = bFrame[3];
		bubbles[3].radius = bubbleRadius[3];
	}
	
	if( b4Frame == 0 )
	{
		bubbles[4].on = false;
	}
	else
	{
		bubbles[4].on = true;
		bubbles[4].pos = bPos[4];
		bubbles[4].frame = bFrame[4];
		bubbles[4].radius = bubbleRadius[4];
	}
	
	//if( b5Frame == 0 )
	//{
	//	bubbles[5].on = false;
	//}
	//else
	//{
	//	bubbles[5].on = true;
	//	bubbles[5].pos = bubble5;
	//	bubbles[5].frame = b5Frame;
	//}
}

void main()
{
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1.0 - fc.y;
	fc = fc * vec2( 1920.0, 1080.0 ) / Resolution;//vec2( 1920, 1080 ) / Resolution;//vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
	
	vec2 fragC = gl_FragCoord.xy;
	fragC.y = 1.0 - fragC.y;
	
	//vec2 pos = pixelPos + topLeft;
	float xtex = gl_TexCoord[0].x;
	float ytex = gl_TexCoord[0].y;
	
	vec4 sum = vec4( 0.0 );
	float blur = 1.0;
	//vec2 tc = gl_TexCoord[0].xy; 
	vec2 tc = gl_FragCoord.xy;//gl_TexCoord[0].xy; 
	
	float hstep = 1.0;
	float vstep = 0.0;
	
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
	
	
	
	for( int i = 0; i < 10; i++ )
	{
		if( bFrame[i] > 0.0 )
		{
			float D = length( bPos[i] * Resolution.xy - vec2( fragC.x, fragC.y) ) * zoom;
			vec2 dir = bPos[i] * Resolution.xy - vec2( fragC.x, fragC.y) * zoom;
			dir = normalize( dir );
			float trueRad = bubbleRadius[i] * 2.0;
			float expandFrames = 20.0;
			float trueFrame  = ( 240.0 - bFrame[i] );
			if( trueFrame < expandFrames )
			{
				trueRad = (trueRad + 80.0 ) * ( 240.0 - bFrame[i] ) / expandFrames;
			}
			else if( trueFrame == expandFrames )
			{
				trueRad = (trueRad + 40.0 ) * ( 240.0 - bFrame[i] - 1.0 ) / expandFrames;
			}
			
			
			if( D <= trueRad )
			{
				if( trueFrame <= expandFrames )
				{
					col.r = 1.0 - col.r;
					col.g = 1.0 - col.g;
					col.b = 1.0 - col.b;
				}
				else
				{
					float gray = dot(col.rgb, vec3(0.299, 0.587, 0.114));
					if( i >= 5 )
					{
						col.r = gray;
						col.b = gray + .5;
					}
					else
					{
						col.b = gray;
						col.r = gray + .5;
					}
					
					col.g = gray;
					
				}
				break;
			}
		}
	}
	
	
	gl_FragColor = col;// * vec4(sum.rgb, 1.0);
}

