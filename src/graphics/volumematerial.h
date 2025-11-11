#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

#include "../framework/camera.h"
#include "material.h"

class VolumeMaterial : public Material {
public:

	float absorption_coefficent;
	float step_length;        // Ray-marching step length (Exercise 3.2)
	int is_homogeneous;       // 1 = homogeneous, 0 = heterogeneous (Exercise 3.2)
	float noise_scale;        // Noise sampling scale (Exercise 3.2)
	glm::vec4 emission_color;  // Emission color (Exercise 3.3)
	float emission_intensity;  // Emission intensity multiplier (Exercise 3.3)
	int shader_mode;          // 0 = Absorption Only (3.1/3.2), 1 = Emission-Absorption (3.3)

	VolumeMaterial(float absorption_coeff = 0.5f);

	void setUniforms(Camera* camera, glm::mat4 model);
	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
	void renderInMenu();
};

