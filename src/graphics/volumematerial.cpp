#include "volumematerial.h"
#include "application.h"


VolumeMaterial::VolumeMaterial(float absorption_coeff)
{
	this->absorption_coefficent = absorption_coeff;
	
	// Exercise 3.2: Initialize heterogeneous volume parameters
	this->step_length = 0.1f;       // Default step length for ray-marching
	this->is_homogeneous = 1;       // Start with homogeneous (1 = homogeneous, 0 = heterogeneous)
	this->noise_scale = 2.0f;       // Default noise scale
	
	// Exercise 3.3: Initialize emission parameters
	this->emission_color = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);  // Default: orange glow
	this->emission_intensity = 1.0f;  // Default emission intensity
	this->shader_mode = 0;          // Start with Absorption Only shader (exercises 3.1 & 3.2)
	
	// Load default shader (Absorption Only)
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
	
	// Upload emission uniforms (Exercise 3.3)
	this->shader->setUniform("u_emission_color", this->emission_color);
	this->shader->setUniform("u_emission_intensity", this->emission_intensity);
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

	// Shader mode selection
	const char* shaderModes[] = { "Absorption Only (Ex 3.1/3.2)", "Emission-Absorption (Ex 3.3)" };
	int previousMode = this->shader_mode;
	ImGui::Combo("Shader Mode", &this->shader_mode, shaderModes, 2);
	
	// Reload shader if mode changed
	if (previousMode != this->shader_mode)
	{
		if (this->shader_mode == 0)
		{
			// Load Absorption Only shader (exercises 3.1 & 3.2)
			this->shader = Shader::Get("res/shaders/volume.vs", "res/shaders/volume.fs");
		}
		else
		{
			// Load Emission-Absorption shader (exercise 3.3)
			this->shader = Shader::Get("res/shaders/volume_emission.vs", "res/shaders/volume_emission.fs");
		}
	}
	
	ImGui::Separator();
	
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
		
		// Exercise 3.3: Emission controls (only visible in Emission-Absorption mode)
		if (this->shader_mode == 1)
		{
			ImGui::Separator();
			ImGui::Text("Emission (Exercise 3.3)");
			ImGui::ColorEdit4("Emission Color", &this->emission_color.r);
			ImGui::DragFloat("Emission Intensity", &this->emission_intensity, 0.1f, 0.0f, 20.0f);
		}
	}
}

