#version 120

uniform sampler2D tex;
uniform vec2 texSize;
uniform vec2 botLeftVel;
uniform float oldZoom;
uniform float zoom;
uniform vec2 oldBotLeft;
uniform vec2 botLeft;
uniform vec4 oldMa;
uniform vec4 ma;
uniform vec2 testVel;


uniform mat4 g_ViewProjectionInverseMatrix;
uniform mat4 g_previousViewProjectionMatrix;

void main()
{
	float numSamples = 3;
	 // Get the initial color at this pixel.  
	vec2 texCoord = gl_FragCoord.xy;
	vec4 color = texture2D(tex, texCoord / texSize ); 

	
	vec4 H = vec4(texCoord.x / texSize.x * 2.0 - 1.0, (1.0 - texCoord.y / texSize.y) * 2.0 - 1.0,  
		1.0, 1.0);  
		
	vec4 D = H * g_ViewProjectionInverseMatrix;  
	// Transform by the view-projection inverse.  
		   
	// Divide by w to get the world position.  
	vec4 worldPos = D / D.w;  
	
	 // Current viewport position  
    vec4 currentPos = H;  
	// Use the world position, and transform by the previous view-  
    // projection matrix.  
    vec4 previousPos = worldPos *g_previousViewProjectionMatrix; //mul(worldPos, g_previousViewProjectionMatrix);  
	// Convert to nonhomogeneous points [-1,1] by dividing by w.  
	previousPos /= previousPos.w;  
	// Use this frame's position and last frame's to compute the pixel  
    // velocity.  
    vec2 velocity = (currentPos.xy - previousPos.xy)/2.0;  
	
	velocity = testVel;
	texCoord += velocity;// / texSize;  
	
	for(int i = 1; i < numSamples; ++i, texCoord += velocity)  
	{  
	  // Sample the color buffer along the velocity vector.  
	   vec4 currentColor = texture2D(tex, texCoord / texSize);  
	   
	  // Add the current color to our color sum.  
	  color += currentColor;  
	}  
	
	// Average all of the samples to get the final blur color.  
	//float4 finalColor = color / numSamples;  
	gl_FragColor = color / numSamples;
}

  