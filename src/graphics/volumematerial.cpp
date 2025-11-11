#include "volumematerial.h"
#include "application.h"


VolumeMaterial::VolumeMaterial(float absorption_coeff)
{
	this->absorption_coefficent = absorption_coeff;
	
	// Exercise 3.2: Initialize heterogeneous volume parameters
	this->step_length = 0.1f;       // Default step length for ray-marching
	this->is_homogeneous = 1;       // Start with homogeneous (1 = homogeneous, 0 = heterogeneous)
	this->noise_scale = 2.0f;       // Default noise scale
	
	// Load volume shaders
	this->shader = Shader::Get("res/shaders/volume.vs", "res/shaders/volume.fs");
}


void VolumeMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	// Upload standard node uniforms
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	this->shader->setUniform("u_camera_position", camera->eye);
	this->shader->setUniform("u_model", model);

	// Upload volume-specific uniforms (Exercise 3.1)
	this->shader->setUniform("u_background_color", Application::instance->ambient_light);
	this->shader->setUniform("u_absorption_coeff", this->absorption_coefficent);
	
	// Upload heterogeneous volume uniforms (Exercise 3.2)
	this->shader->setUniform("u_step_length", this->step_length);
	this->shader->setUniform("u_is_homogeneous", this->is_homogeneous);
	this->shader->setUniform("u_noise_scale", this->noise_scale);
}

void VolumeMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	if (mesh && this->shader) {
		this->shader->enable();

		setUniforms(camera, model);

		mesh->render(GL_TRIANGLES);

		this->shader->disable();
	}
}

void VolumeMaterial::renderInMenu()
{
	ImGui::Text("Material Type: %s", std::string("Volume").c_str());

	// Exercise 3.1: Absorption coefficient control
	ImGui::DragFloat("Absorption Coefficient", &this->absorption_coefficent, 0.01f, 0.0f, 10.0f);
	
	// Exercise 3.2: Volume type combo box
	const char* volumeTypes[] = { "Heterogeneous", "Homogeneous" };
	ImGui::Combo("Volume Type", &this->is_homogeneous, volumeTypes, 2);
	
	// Exercise 3.2: Show heterogeneous-specific controls only when heterogeneous is selected
	if (this->is_homogeneous == 0)  // 0 = heterogeneous
	{
		ImGui::DragFloat("Step Length", &this->step_length, 0.01f, 0.01f, 1.0f);
		ImGui::DragFloat("Noise Scale", &this->noise_scale, 0.1f, 0.1f, 10.0f);
	}
}

