// front.frag

#version 300 es

#define SURFACE 0		// renderType
#define DRR     1
#define MIP     2

precision highp float;

uniform sampler2D texture_fbo; //The frame buffer object to the G-buffer, back coords render 
uniform sampler3D texture_volume;   //tau values 
uniform sampler3D texture_gradient; //normalized normals   

uniform float fbWidth;  //frambuffer w
uniform float fbHeight; //frambuffer h
uniform float slice_spacing;
uniform float densityFactor;
uniform float shininess;

uniform int   renderType; //as shown above 
uniform int   invert;     
uniform int   useSpecular;

uniform vec3 light_direction;
uniform vec3 lightColour;
uniform vec3 volumeScale; //scales for each dimensions of volume should scaled 

uniform mat4 MVinverse;

in vec3 texCoords;		// texCoords on front face of volume
in vec3 fragPosition;		// fragment position in VCS

out vec4 fragColour;



void main()

{
  // Get cube's front texCoords 

  // vec3 frontCoords = vec3(0,1,0);  // [YOUR CODE HERE]
  vec3 frontCoords = texCoords;

#if 0 // testing
  fragColour = vec4( frontCoords, 1 );
  return;
#endif

  // Compute texture coordinate of this fragment in the 2D FBO.  These
  // texture coordinates are in [0,1]x[0,1].
  //
  // Use the implicitly-defined 'gl_FragCoord', which is the PIXEL
  // POSITION of the fragment in the framebuffer in the range
  //
  //    [0,width-1] x [0,height-1]
  //
  // where 'width' and 'height' are the dimensions of the framebuffer.

  // vec2 fboCoords = vec2( 0, 0 ); // [YOUR CODE HERE]
  // vec2 fboCoords = vec2( gl_FragCoord.x * 1.0f / fbWidth, gl_FragCoord.y * 1.0f / fbHeight);
  vec2 fboCoords = vec2( gl_FragCoord.x / fbWidth, gl_FragCoord.y / fbHeight);

#if 0 // testing.  zoom in to make the bounding volume fill the whole screen.
  fragColour  = vec4( fboCoords.x, fboCoords.y, 0, 1 );
  return;
#endif

  // Get cube's back texCoords
  //
  // These are stored in the FBO texture.

  // vec3 backCoords = vec3(0,0,1); // [YOUR CODE HERE]
  vec3 backCoords = texture(texture_fbo, fboCoords).rgb;

#if 0 // testing
  fragColour = vec4( backCoords, 1 );
  return;
#endif

  // determine front-to-back distance in WORLD COORDINATES.  You'll
  // have to scale the [0,1]x[0,1]x[0,1] texture coordinates by
  // 'volumeScale' to get the world coordinates, and then compute the
  // distance.

  // float dist = 0.5;  // [YOUR CODE HERE]
  vec3 WFC = frontCoords * volumeScale;
  vec3 WBC = backCoords * volumeScale;
  vec3 WDiffC = WFC - WBC;
  float dist =  sqrt(pow(WDiffC.x,2.0f)+pow(WDiffC.y,2.0f)+pow(WDiffC.z,2.0f));

#if 0 // testing.  You should see different distances in the bounding volume.
  fragColour = vec4( dist, dist, dist, 1 );
  return;
#endif

  // Determine the NORMALIZED viewing direction along this ray back
  // toward the eye.  Then move this into the OCS, since the gradients
  // (i.e. surface normals) are in the OCS and all lighting
  // calculations should take place in the OCS.

  // vec3 viewDir = vec3(1,0,0);
  vec3 viewDir = normalize((MVinverse * vec4(-fragPosition,0.0)).rgb);

#if 0 // testing
  fragColour = vec4( 0.5 * (viewDir + vec3(1,1,1)), 1 ); //test for mapping [-1,1]to [0,1]
  return;
#endif

  // Step front-to-back, accumulating colour
  //
  // Use the correct spacing in texture coordinates.
  //
  // See the lecture notes for details here.
  //
  // When computing alpha at a position inside the volume, MULTIPLY IT
  // BY THE UNIFORM 'densityFactor', which the user controls by
  // pressing '+' or '-' to make things appear more or less dense.

  float trans = 1.0;

#if 1

  int numInterval = int(dist/slice_spacing); 
  vec3 deltaS = (backCoords - frontCoords) / float(numInterval);

  //init 
  float tau = 0.0f;
  float alpha = 0.0f;
  float maxAlpha = -1.0f;   //for MIP 
  float taui = 1.0f;      //total transparency  

  vec3 Iout = vec3( 0.0f, 0.0f, 0.0f );
  vec3 C = vec3(0.0, 0.0,  0.0);
  vec3 kd = vec3(1.0, 1.0, 1.0);
  vec3 ks = vec3(1.0, 1.0, 1.0);
  vec3 backLighting = vec3(1.0, 1.0, 1.0);
  mat4 MV = inverse(MVinverse);

  for ( int n = 0; n <= numInterval; n++ ){

    // Gather tau, alpha, and the gradient

    // [YOUR CODE HERE]
    vec3 intervalCoords = frontCoords + float(n) * deltaS; 
    tau = texture(texture_volume, intervalCoords).x;
    // float alpha = tau * length(deltaS) * densityFactor;     // alpha = Delta s * tau 
    alpha = tau * length(deltaS) * densityFactor;     // alpha = Delta s * tau 
    maxAlpha = max(maxAlpha, alpha);                        // Update max alpha for MIP  

    vec3 N = texture(texture_gradient, intervalCoords).rgb; //gradient
    N = normalize((MV * vec4(N,0.0f)).rgb); 


    // Compute C based on the current 'renderType', which is one of
    // MIP, DRR, or SURFACE.
    //
    // Initially, do this only for the DRR, which is the simplest
    // computation.  (See the lecture notes on this.)

    if(renderType == DRR){

      C = vec3(0.0, 0.0, 0.0);

    }else if(renderType == SURFACE){
      //phong calculation
      //first convert all compnent into WCS and normalize them 
      mediump vec3 light_direction_wcs = normalize((MV * vec4(light_direction,0.0f)).rgb);
      mediump vec3 viewDir_wcs = normalize((MV * vec4(viewDir,0.0f)).rgb);

      //specular and diffuse calculation 
      mediump float NdotL = max(dot(N, light_direction_wcs), 0.0f);
      mediump vec3 R = (2.0f * NdotL) * N - light_direction_wcs;
      vec3 Is = ks * pow(max(dot(viewDir_wcs, R), 0.0f), shininess); //for specular calculation
      vec3 Id = kd * NdotL; //for diffuse calculation
      C = lightColour * (Id + Is);
    }
    // [YOUR CODE HERE]


    // Add this position's C to the accumulated colour, including the
    // appropriate alpha.
    //
    // How you do this will depend on the 'renderType'.
    Iout = Iout + taui * C * alpha;
    taui = taui * (1.0f - alpha);
  }

  if(renderType == MIP){
      Iout = vec3(maxAlpha);
  }
  Iout = Iout + taui * backLighting;

#endif

  // Output

  if (invert == 1)
     Iout = vec3(1,1,1) - Iout;

  fragColour = vec4( Iout, 1 );
}
