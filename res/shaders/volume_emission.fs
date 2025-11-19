#version 330 core

in vec3 v_world_position;

uniform vec3 u_camera_position;
uniform mat4 u_model;
uniform vec4 u_background_color;
uniform float u_absorption_coeff;
uniform float u_step_length;      // Step length for ray-marching (Exercise 3.2)
uniform int u_is_homogeneous;     // 1 = homogeneous, 0 = heterogeneous (Exercise 3.2)
uniform float u_noise_scale;      // Scale for noise sampling (Exercise 3.2)
uniform vec4 u_emission_color;    // Emission color (Exercise 3.3)
uniform float u_emission_intensity; // Emission intensity multiplier (Exercise 3.3)
uniform sampler3D u_volume_texture;  // 3D texture from VDB (Lab 4, Task 3.1)
uniform int u_volume_source;         // 0=VDB, 1=Noise, 2=Constant (Lab 4, Task 3.1)

out vec4 FragColor;

// Simplex 3D Noise 
// by Ian McEwan, Ashima Arts
// Reference: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

float snoise(vec3 v){ 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //  x0 = x0 - 0. + 0.0 * C 
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

// Permutations
  i = mod(i, 289.0 ); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients
// ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0/7.0; // N=7
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
}

// adapted from intersectCube in https://github.com/evanw/webgl-path-tracing/blob/master/webgl-path-tracing.js
// compute the near and far intersections of the cube (stored in the x and y components) using the slab method
// no intersection means vec.x > vec.y (really tNear > tFar)
vec2 intersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) {
	vec3 tMin = (boxMin - rayOrigin) / rayDir;
	vec3 tMax = (boxMax - rayOrigin) / rayDir;
	vec3 t1 = min(tMin, tMax);
	vec3 t2 = max(tMin, tMax);
	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);
	return vec2(tNear, tFar);
}

void main()
{
	// Step 1: Initialize ray (position and direction)
	vec3 rayOrigin = u_camera_position;
	vec3 rayDir = normalize(v_world_position - u_camera_position);
	
	// Transform ray to object space for intersection test
	mat4 invModel = inverse(u_model);
	vec3 rayOriginObj = (invModel * vec4(rayOrigin, 1.0)).xyz;
	vec3 rayDirObj = normalize((invModel * vec4(rayDir, 0.0)).xyz);
	
	// Step 2: Compute intersections with volume auxiliary geometry
	// Cube mesh is created from -1 to +1 in object space: [-1, -1, -1] to [1, 1, 1]
	vec3 boxMin = vec3(-1.0, -1.0, -1.0);
	vec3 boxMax = vec3(1.0, 1.0, 1.0);
	
	vec2 intersection = intersectAABB(rayOriginObj, rayDirObj, boxMin, boxMax);
	float ta = intersection.x;  // tNear (entry point)
	float tb = intersection.y;  // tFar (exit point)
	
	
	float opticalThickness = 0.0;
	vec4 accumulatedEmission = vec4(0.0);
	float transmittance = 1.0;  // Start with full transmittance
	
	// Step 3: Compute optical thickness and emission
	if (u_is_homogeneous == 1)
	{
		// HOMOGENEOUS VOLUME (Exercise 3.1)
		// For homogeneous volumes: τ = distance * μa
		// NOTE: Emission model only works for heterogeneous volumes
		float distanceObj = tb - ta;
		vec3 distVecObj = rayDirObj * distanceObj;
		vec3 distVecWorld = (u_model * vec4(distVecObj, 0.0)).xyz;
		float distanceWorld = length(distVecWorld);
		
		opticalThickness = distanceWorld * u_absorption_coeff;
		transmittance = exp(-opticalThickness);
	}
	else
	{
		// HETEROGENEOUS VOLUME WITH EMISSION (Exercise 3.2 + 3.3)
		// Ray-marching to accumulate both optical thickness and emission
		
		// Pre-compute world-space step length
		vec3 stepVecObj = rayDirObj * u_step_length;
		vec3 stepVecWorld = (u_model * vec4(stepVecObj, 0.0)).xyz;
		float stepLengthWorld = length(stepVecWorld);
		
		// Ray-marching loop
		float t = ta;
		while (t < tb)
		{
		// Current position in object space
		vec3 posObj = rayOriginObj + rayDirObj * t;
		
		// Sample density based on selected volume source (Lab 4, Task 3.1)
		float density;
		if (u_volume_source == 0) {
			// VDB: Sample from 3D texture
			vec3 texCoords = (posObj + 1.0) * 0.5;  // Transform [-1,1] to [0,1]
			density = texture(u_volume_texture, texCoords).r;
		} else if (u_volume_source == 1) {
			// 3D Noise (existing Lab 3 implementation)
			density = max(snoise(posObj * u_noise_scale), 0.0);
		} else {
			// Constant density (uses absorption coefficient as constant value)
			density = u_absorption_coeff;
		}
			
			// Compute optical thickness for this step
			float stepOpticalThickness = density * u_absorption_coeff * stepLengthWorld;
			
			// Compute local transmittance for this step
			float localTransmittance = exp(-stepOpticalThickness);
			
			// Update total transmittance
			transmittance *= localTransmittance;
			
			// Accumulate emission (before updating transmittance)
			// Emission contribution = density * emission_color * current_transmittance * step_length
			vec4 emissionContribution = density * u_emission_color * u_emission_intensity * transmittance * stepLengthWorld;
			accumulatedEmission += emissionContribution;
			
			
			// Advance along the ray
			t += u_step_length;
		}
	}
	
	// Step 4: Compute final radiance
	// L_final = accumulated_emission + background * transmittance
	FragColor = accumulatedEmission + u_background_color * transmittance;
}

