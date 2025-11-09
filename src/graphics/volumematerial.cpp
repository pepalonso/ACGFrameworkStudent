#include "volumematerial.h"
#include "application.h"


VolumeMaterial::VolumeMaterial(float absorption_coeff)
{
	this->absorption_coefficent = absorption_coeff;
	// Load volume shaders
	this->shader = Shader::Get("res/shaders/volume.vs", "res/shaders/volume.fs");
}


void VolumeMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	// Upload standard node uniforms
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	this->shader->setUniform("u_camera_position", camera->eye);
	this->shader->setUniform("u_model", model);

	// Upload volume-specific uniforms
	this->shader->setUniform("u_background_color", Application::instance->ambient_light);
	this->shader->setUniform("u_absorption_coeff", this->absorption_coefficent);
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

	ImGui::DragFloat("Absorption Coefficient", &this->absorption_coefficent, 0.01f, 0.0f, 10.0f);
}

