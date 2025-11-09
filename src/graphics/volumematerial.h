#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

#include "../framework/camera.h"
#include "material.h"

class VolumeMaterial : public Material {
public:

	float absorption_coefficent;

	VolumeMaterial(float absorption_coeff = 0.5f);

	void setUniforms(Camera* camera, glm::mat4 model);
	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
	void renderInMenu();
};

