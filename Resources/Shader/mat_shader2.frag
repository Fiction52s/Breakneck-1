#version 120

uniform sampler2D u_texture;
uniform sampler2D u_normals;   //normal map
uniform sampler2D u_pattern;

uniform vec4 skyColor;
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
	float size = 512.0;
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1.0 - fc.y;
	fc = fc * vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );	
	
	vec2 pos = mod( topLeft + pixelPos, size) / vec2( size );
	
	vec4 finalfinal = vec4( 0.0, 0.0, 0.0, 0.0 );
	
	vec4 DiffuseColor;
	
	DiffuseColor = texture2D(u_texture, pos );
	
	vec3 Ambient = AmbientColor.rgb * AmbientColor.a;
	vec3 Intensity = Ambient;//.000001;
	finalfinal = vec4( DiffuseColor.rgb * Intensity, DiffuseColor.a );
	
	finalfinal = gl_Color * finalfinal;
	
	finalfinal.rgb = vec3(.9) * finalfinal.rgb + vec3( .1 ) * skyColor.rgb;
	
	gl_FragColor = finalfinal;
}

