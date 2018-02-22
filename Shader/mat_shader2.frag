#version 120

uniform sampler2D u_texture;
uniform sampler2D u_normals;   //normal map
uniform sampler2D u_pattern;

uniform vec2 topLeft;
uniform float zoom;

//layout(origin_upper_left) in vec4 gl_FragCoord;

//this makes things weird sometimes. need y coords reversed in a way.

//values used for shading algorithm...
uniform vec2 Resolution;      //resolution of screen

uniform vec3 LightPos0;        //light position, normalized
uniform vec4 LightColor0;      //light RGBA -- alpha is intensity
uniform float Brightness0;
uniform float Radius0;
uniform bool On0;

uniform vec3 LightPos1;        
uniform vec4 LightColor1;      
uniform float Brightness1;
uniform float Radius1;   
uniform bool On1;

uniform vec3 LightPos2;        
uniform vec4 LightColor2;      
uniform float Brightness2;
uniform float Radius2;
uniform bool On2;

uniform vec3 LightPos3;        
uniform vec4 LightColor3;
uniform float Brightness3;
uniform float Radius3;
uniform bool On3;

uniform vec3 LightPos4;
uniform vec4 LightColor4;
uniform float Brightness4;
uniform float Radius4;
uniform bool On4;

uniform vec3 LightPos5;
uniform vec4 LightColor5;
uniform float Brightness5;
uniform float Radius5;
uniform bool On5;

uniform vec3 LightPos6;
uniform vec4 LightColor6;
uniform float Brightness6;
uniform float Radius6;
uniform bool On6;

uniform vec3 LightPos7;
uniform vec4 LightColor7;
uniform float Brightness7;
uniform float Radius7;
uniform bool On7;

uniform vec3 LightPos8;
uniform vec4 LightColor8;
uniform float Brightness8;
uniform float Radius8;
uniform bool On8;

uniform vec3 LightPosPlayer;
uniform vec4 LightColorPlayer;
uniform float BrightnessPlayer;
uniform float RadiusPlayer;

vec2 oldBotLeft;
float oldZoom;
vec2 playertest;

uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 


const int numLights = 0;
struct LightSource
{
	bool on;
	vec3 pos;
	vec4 color;
	float brightness;
	float radius;
};
LightSource lights[1];



void main()
{

	//InitLights();
	
	float size = 512.0;
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1.0 - fc.y;
	fc = fc * vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
	//vec2 pixelPos = vec2( fc.x / zoom, fc.y / zoom );
	
	
	vec2 pos = mod( topLeft + pixelPos, size) / vec2( size );
	//gl_FragColor = texture2D( u_texture, pos );
	
	vec4 finalfinal = vec4( 0.0, 0.0, 0.0, 0.0 );
	
	
	//if( numLightsOn == 0 )
	{
		//vec2 diff = topLeft - oldBotLeft;
		//diff = normalize( diff );
		//diff = diff / vec2( 3 );
		vec2 pos2 = pos;
		//pos2.x = pos2.x + diff.x;
		//pos2.y = pos2.y + diff.y;
		
		pos2.x = pos2.x - .1;//playertest.x;
		pos2.y = pos2.y + playertest.y;
		pos2.x = float( pos2.x < 0 ) * 1.0 + pos2.x;
		pos2.y = float( pos2.y < 0 ) * 1.0 + pos2.y;
		
		vec4 DiffuseColor;
		
		//int max = 5;
		//for( int i = 0; i < max; i++ )
		//{
		//	float div = i / float(max);
		//	DiffuseColor += texture2D(u_texture, pos * ( 1 - div ) + pos2 * div ) / vec4(max);
		//}
		 //= (texture2D(u_texture, pos) * vec4(.5) + texture2D(u_texture, pos2) * vec4(.5));
		
		DiffuseColor = texture2D(u_texture, pos );
		
		vec3 Ambient = AmbientColor.rgb * AmbientColor.a;
		vec3 Intensity = Ambient;//.000001;
		finalfinal = vec4( DiffuseColor.rgb * Intensity, DiffuseColor.a );
	}
	gl_FragColor =  gl_Color * finalfinal;//vec4(finalfinal, DiffuseColor.a);
	
	
}

