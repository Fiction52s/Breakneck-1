uniform sampler2D zoomTex;
uniform vec2 texSize;
uniform float radial_blur;   // blur factor
uniform float radial_bright; // bright factor
uniform vec2 radial_origin;  // blur origin
uniform vec2 radial_size;
//varying vec2 uv;


/*void main()
{
	vec2 TexCoord = vec2(gl_TexCoord[0].xy);
	gl_FragColor = texture2D(zoomTex, TexCoord );//* texSize );
}*/

/*void main(void)
{
  vec2 TexCoord = vec2(gl_TexCoord[0].st) / texSize;//vec2(  gl_FragCoord.xy / texSize;//vec2(gl_TexCoord[0]);

  vec4 SumColor = vec4(0.0, 0.0, 0.0, 0.0);
  TexCoord += radial_size * 0.5 - radial_origin;

  for (int i = 0; i < 12; i++) 
  {
    float scale = 1.0 - radial_blur * (float(i) / 11.0);
    SumColor += texture2D(zoomTex, (TexCoord * scale + radial_origin) * texSize);
  }

  gl_FragColor = SumColor / 12.0;// * radial_bright;
}*/

//uniform sampler2D tex;
//varying vec2 uv;
const float sampleDist = 1.0;
uniform float sampleStrength;// = 5.2; 

void main(void)
{
	vec2 TexCoord =  vec2(gl_TexCoord[0]);//gl_FragCoord.xy / texSize;//vec2(gl_TexCoord[0]);//gl_FragCoord.xy / texSize;//vec2(gl_TexCoord[0]);

    float samples[10];
    samples[0] = -0.08;
    samples[1] = -0.05;
    samples[2] = -0.03;
    samples[3] = -0.02;
    samples[4] = -0.01;
    samples[5] =  0.01;
    samples[6] =  0.02;
    samples[7] =  0.03;
    samples[8] =  0.05;
    samples[9] =  0.08;

    vec2 dir = 0.5 - TexCoord; 
    float dist = sqrt(dir.x*dir.x + dir.y*dir.y); 
    dir = dir/dist; 

    vec4 color = texture2D(zoomTex,TexCoord); 
    vec4 sum = color;

    for (int i = 0; i < 10; i++)
        sum += texture2D( zoomTex, TexCoord + dir * samples[i] * sampleDist );

    sum *= 1.0/11.0;
    float t = dist * sampleStrength;
    t = clamp( t ,0.0,1.0);

    gl_FragColor = mix( color, sum, t );
}