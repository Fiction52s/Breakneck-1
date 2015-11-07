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


uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 


const int numLights = 10;
struct LightSource
{
	bool on;
	vec3 pos;
	vec4 color;
	float brightness;
	float radius;
};
LightSource lights[numLights];

void InitLights()
{
	lights[0].on = On0;
	lights[0].pos = LightPos0;
	lights[0].color = LightColor0;//vec4( 1, 0, 0, 1 );//LightColor;
	lights[0].brightness = Brightness0;
	lights[0].radius = Radius0;
	
	lights[1].on = On1;
	lights[1].pos = LightPos1 ;// + vec3( .1, 0, 0 ) / zoom;
	lights[1].color = LightColor1; //vec4( 0, 1, 0, 1 );
	lights[1].brightness = Brightness1;
	lights[1].radius = Radius1;
	
	lights[2].on = On2;
	lights[2].pos = LightPos2;// + vec3( .05, .05, 0 ) / zoom ;
	lights[2].color = LightColor2; //vec4( 0, 0, 1, 1 );
	lights[2].brightness = Brightness2;
	lights[2].radius = Radius2;
	
	lights[3].on = On3;
	lights[3].pos = LightPos3;
	lights[3].color = LightColor3;
	lights[3].brightness = Brightness3;
	lights[3].radius = Radius3;
	
	lights[4].on = On4;
	lights[4].pos = LightPos4;
	lights[4].color = LightColor4;
	lights[4].brightness = Brightness4;
	lights[4].radius = Radius4;
	
	lights[5].on = On5;
	lights[5].pos = LightPos5;
	lights[5].color = LightColor5;
	lights[5].brightness = Brightness5;
	lights[5].radius = Radius5;
	
	lights[6].on = On6;
	lights[6].pos = LightPos6;
	lights[6].color = LightColor6;
	lights[6].brightness = Brightness6;
	lights[6].radius = Radius6;
	
	lights[7].on = On7;
	lights[7].pos = LightPos7;
	lights[7].color = LightColor7;
	lights[7].brightness = Brightness7;
	lights[7].radius = Radius7;
	
	lights[8].on = On8;
	lights[8].pos = LightPos8;
	lights[8].color = LightColor8;
	lights[8].brightness = Brightness8;
	lights[8].radius = Radius8;
	
	lights[9].on = true;
	lights[9].pos = LightPosPlayer;
	lights[9].color = LightColorPlayer;
	lights[9].brightness = BrightnessPlayer;
	lights[9].radius = RadiusPlayer;
}

void main()
{

	InitLights();
	
	float size = 128.0;
	float patternSize = 16.0;
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1 - fc.y;
	fc = fc * vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
	//vec2 pixelPos = vec2( fc.x / zoom, fc.y / zoom );
	
	vec2 patternPos = mod( ((topLeft + pixelPos) / vec2(size)), patternSize ) / vec2( patternSize );
	vec4 patternColor = texture2D( u_pattern, patternPos );
    vec2 pos;
	if( patternColor == vec4( 1, 1, 1, 1 ) )
	{
		pos = mod( topLeft + pixelPos, size) / vec2( size );
		pos.x = pos.x / 2.0;
	}
	else// if( patternColor == vec4( 0, 0, 0, 0 ) )
	{
		pos = mod( topLeft + pixelPos, size) / vec2( size );
		pos.x = pos.x / 2.0;
		pos.x = pos.x + .5;
	}
	//vec2 pos = mod( topLeft + pixelPos, size) / vec2( size );
	//gl_FragColor = texture2D( u_texture, pos );
	
	vec4 finalfinal = vec4( 0, 0, 0, 0 );
	
	
	
	int numLightsOn = 0;
	for( int i = 0; i < numLights; ++i )
	{
		if( lights[i].on )
			numLightsOn = numLightsOn + 1;
	}
	
	for( int i = 0; i < numLights; ++i )
	{
		if( !lights[i].on )
		{
			continue;
		}
		//noLights = false;
		vec4 DiffuseColor = texture2D(u_texture, pos);
		vec3 NormalMap = texture2D(u_normals, pos).rgb;
		vec2 fragC = gl_FragCoord.xy;
		fragC.y = 1 - fragC.y;
		//vec3 LightDir = vec3(lights[i].pos.xy - (vec2( fragC.x, fragC.y) / Resolution.xy), lights[i].pos.z);
		//LightDir *= Resolution.x / Resolution.y * zoom;
		//LightDir.x *= Resolution.x / Resolution.y;
		float z = lights[i].pos.z;
		vec3 LightDir = vec3(lights[i].pos.xy * Resolution.xy - vec2( fragC.x, fragC.y), z );
		LightDir *= zoom;
		float D = length(LightDir);
		
		float radius = lights[i].radius*2;
		float brightness = lights[i].brightness/4;
		float Attenuation = clamp( 1.0 - (D*D) / (radius*radius), 0.0, 1.0 ); Attenuation *= Attenuation * brightness;	
	
		vec3 N = normalize(NormalMap * 2.0 - 1.0);
		vec3 L = normalize(LightDir);
		vec3 Diffuse = (lights[i].color.rgb * lights[i].color.a) * max(dot(N, L), 0.0);
		vec3 Ambient = AmbientColor.rgb * AmbientColor.a / numLightsOn;
		
		vec3 Intensity = Ambient + Diffuse * Attenuation;
		vec3 FinalColor = DiffuseColor.rgb * Intensity;
		
		finalfinal += vec4( FinalColor, DiffuseColor.a );
	}
	
	if( numLightsOn == 0 )
	{
		vec4 DiffuseColor = texture2D(u_texture, pos);
		
		vec3 Ambient = AmbientColor.rgb * AmbientColor.a;
		vec3 Intensity = Ambient;//.000001;
		finalfinal = vec4( DiffuseColor.rgb * Intensity, DiffuseColor.a );
	}
	gl_FragColor =  gl_Color * finalfinal;//vec4(finalfinal, DiffuseColor.a);
	
	
}

