//advection fragment program

//output a color, which is the advected quantity

#version 120

uniform sampler2D u; //input velocity
uniform sampler2D x; //quantity to advect
uniform float rdx; //1 / grid scale
uniform float timestep;
uniform vec2 texSize;

vec2 colorToVec( vec4 col )
{
	vec2 temp = col.xy;
	temp = (temp - .5) * 2;
	temp = normalize( temp );
	float mag = col.z * 256.0;
	
	temp *= vec2( mag ); //magnitude
	return temp;
}

vec3 vecToColorRGB( vec2 ve )
{
	float len = length( ve );
	vec3 temp = vec3( normalize(ve), 1 );//len / 256.0 );
	temp.xy = (temp.xy * .5) + .5;
	return temp;
}

vec4 f4texRECTbilerp(sampler2D tex, vec2 blah)
{
  vec2 s = blah;
 

  vec4 st;
  st.xy = floor(s - 0.5) + 0.5;
  st.zw = st.xy + 1;
  
  
  
  vec2 t = s - st.xy; //interpolating factors 
    
  vec4 tex11 = texture2D(tex, st.xy / texSize);
  vec4 tex21 = texture2D(tex, st.zy / texSize);
  vec4 tex12 = texture2D(tex, st.xw / texSize);
  vec4 tex22 = texture2D(tex, st.zw / texSize);

  t /= texSize;
  
  // bilinear interpolation
  return mix(mix(tex11, tex21, t.x), mix(tex12, tex22, t.x), t.y);
}

vec4 cubic(float v)
{
    vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w);
}

vec4 textureBicubic(sampler2D sampler, vec2 texCoords){

   //vec2 texSize = textureSize(sampler, 0);
   vec2 invTexSize = 1.0 / texSize;
   
   texCoords = texCoords * texSize - 0.5;

   
    vec2 fxy = fract(texCoords);
    texCoords -= fxy;

    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);

    vec4 c = texCoords.xxyy + vec2(-0.5, 1.5).xyxy;
    
    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4(xcubic.yw, ycubic.yw) / s;
    
    offset *= invTexSize.xxyy;
    
    vec4 sample0 = texture2D(sampler, offset.xz);
    vec4 sample1 = texture2D(sampler, offset.yz);
    vec4 sample2 = texture2D(sampler, offset.xw);
    vec4 sample3 = texture2D(sampler, offset.yw);

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return mix(
       mix(sample3, sample2, sx), mix(sample1, sample0, sx)
    , sy);
}

void main()
{
	vec4 velColor = (texture2D(u, gl_FragCoord.xy / texSize ));//vec4( 1, .5, .5, 1 );//(texture2D(u, gl_FragCoord.xy / texSize ));
	vec2 vel = colorToVec( velColor );
	//vel.x = 128;
	//vel.y = 0;
	
	//follow the vector field back in time
	vec2 pos = gl_FragCoord.xy - vec2(timestep) * vel;
	//vec2 pos = gl_FragCoord.xy - vec2(1) * vec2(rdx) * vel;
	gl_FragColor = f4texRECTbilerp( x, pos );
}