#version 330 core

in vec3 v_world_position;

uniform vec3 u_camera_position;
uniform mat4 u_model;
uniform vec4 u_background_color;
uniform float u_absorption_coeff;

out vec4 FragColor;

// Ray-box intersection function
// Returns true if ray intersects box, false otherwise
// boxMin and boxMax define the axis-aligned bounding box in object space
// rayOriginObj and rayDirObj should be in object space
// Returns intersection distances in object space
bool intersectRayBox(vec3 rayOriginObj, vec3 rayDirObj, vec3 boxMin, vec3 boxMax, out float tNear, out float tFar)
{
	// Compute intersections with each pair of parallel planes
	vec3 invDir = 1.0 / rayDirObj;
	vec3 t0 = (boxMin - rayOriginObj) * invDir;
	vec3 t1 = (boxMax - rayOriginObj) * invDir;
	
	// Find near and far intersection points
	vec3 tMin = min(t0, t1);
	vec3 tMax = max(t0, t1);
	
	// Find the largest near and smallest far
	float largestNear = max(max(tMin.x, tMin.y), tMin.z);
	float smallestFar = min(min(tMax.x, tMax.y), tMax.z);
	
	// Check if ray intersects the box
	if (largestNear > smallestFar || smallestFar < 0.0)
		return false;
	
	tNear = max(largestNear, 0.0);  // Entry point (ta) in object space
	tFar = smallestFar;              // Exit point (tb) in object space
	
	return true;
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
	
	float ta, tb;
	bool hit = intersectRayBox(rayOriginObj, rayDirObj, boxMin, boxMax, ta, tb);
	
	if (!hit)
	{
		// Ray doesn't intersect volume - return background color
		FragColor = u_background_color;
		return;
	}
	
	// Step 3: Compute the optical thickness
	// For homogeneous volumes: τ = (tb - ta) * μa
	// Compute Euclidean distance in world space
	float distanceObj = tb - ta;
	vec3 distVecObj = rayDirObj * distanceObj;
	vec3 distVecWorld = (u_model * vec4(distVecObj, 0.0)).xyz;
	float distanceWorld = length(distVecWorld);
	
	float opticalThickness = distanceWorld * u_absorption_coeff;
	
	// Step 4: Compute the transmittance (Beer-Lambert Law)
	float transmittance = exp(-opticalThickness);
	
	// Step 5: Compute and set the final pixel color
	// L(t) = B * e^(-(tb - ta) * μa) = B * transmittance
	FragColor = u_background_color * transmittance;
}

