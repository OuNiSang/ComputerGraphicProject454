// Pass 2 fragment shader
//
// Apply diffuse lighting to fragment.  Later do Phong lighting.
//
// Determine whether fragment is in shadow.  If so, reduce intensity to 50%.

#version 300 es
precision mediump float;


uniform vec3      lightDir;	    // direction to light in WCS
uniform vec3      eyePosition;	    // position of eye in WCS
uniform mat4      WCS_to_lightCCS;  // transform from WCS to light's CCS
uniform sampler2D shadowBuffer;     // texture [0,1]x[0,1] of depth from light.  Values in [0,1].
uniform sampler2D objTexture;       // object's texture (might not be provided)
uniform bool      texturing;        // =1 if object's texture is provided


in vec3 colour;        // fragment colour
in vec3 normal;        // fragment normal in WCS
in highp vec3 wcsPosition;   // fragemnt position in WCS
in vec2 texCoords;     // fragment texture coordinates (if provided) 

out vec4 fragColour;   // fragment's final colour


uniform vec3 Iin;
//uniform vec3 kd;
uniform vec3 ks;
uniform vec3 Ia;
uniform vec3 Ie;
uniform float shininess;

void main()

{
  //Self phong define: 
  mediump vec3	N		= normalize(normal);
  mediump float NdotL	= dot(N, lightDir);
  mediump vec3	kd		= colour;

  // Calculate the position of this fragment in the light's CCS.

  vec4 ccsLightPos = WCS_to_lightCCS * vec4(wcsPosition,1.0f); // CHANGE THIS

  // Calculate the depth of this fragment in the light's CCS in the range [0,1]
  
  float fragDepth = (ccsLightPos.z + 1.0f) * 0.5f; // CHANGE THIS

  // Determine the (x,y) coordinates of this fragment in the light's
  // CCS in the range [0,1]x[0,1].

  vec2 shadowTexCoords = vec2((ccsLightPos / ccsLightPos.w + 1.0f)) * 0.5f; // CHANGE THIS

  // Look up the depth from the light in the shadowBuffer texture.
//  float bias = 0.005;
  float bias = max(0.005 * (1.0 - NdotL), 0.001);
  float shadowDepth = texture(shadowBuffer, shadowTexCoords).r + bias; // CHANGE THIS 

  // Determine whether the fragment is in shadow.
  //
  // If results look bad, add a bit to the shadow texture depth to
  // prevent z-fighting.

  // YOUR CODE HERE
  float shadowValue = 1.0f;
  if(fragDepth > shadowDepth) //This means this fragment is in shadow 
	shadowValue = 0.5f;

  // Choose the colour either from the object's texture (if
  // 'texturing' == 1) or from the input colour.

  // YOUR CODE HERE

  if(texturing){
	kd = texture(objTexture,texCoords).rgb;}
	
  // Compute illumination.  Initially just do diffuse "N dot L".  Later do Phong.

  // YOUR CODE HERE

  //----Emissive 
  mediump vec3 Iout = Ie;

  //----Ambient
  Iout += Ia;

  //----Defuse
  
  Iout += NdotL * vec3(kd.r * Iin.r, kd.g * Iin.g, kd.b * Iin.b) * shadowValue; 

  //----Specular
  mediump vec3 R = (2.0f * NdotL) * N - lightDir;
  mediump vec3 V = normalize(eyePosition - wcsPosition);
  mediump float RdotV = max(dot(R,V),0.0); 
  Iout += pow(RdotV, shininess) * vec3(ks.r * Iin.r, ks.g * Iin.g, ks.b * Iin.b) * shadowValue;

  // Output the fragment colour, modified by the illumination model
  // and shadowing.
  
  fragColour = vec4(Iout, 1.0f);	// CHANGE THIS
}
