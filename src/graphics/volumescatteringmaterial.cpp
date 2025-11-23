#include "volumescatteringmaterial.h"
#include "application.h"

VolumeScatteringMaterial::VolumeScatteringMaterial(float absorption_coeff)
	: VolumeMaterial(absorption_coeff)  // Call parent constructor
{
	// Lab 4 defaults
	this->volume_source = 1;        // Start with Noise: 0=VDB, 1=Noise, 2=Constant
	this->is_homogeneous = 0;       // Heterogeneous (ray-marching required for VDB)
	this->shader_mode = 1;          // Emission-Absorption mode
	
	// Task 3.2: Initialize scattering parameters
	this->scattering_coeff = 0.5f;      // Default scattering coefficient μs
	this->light_step_length = 0.1f;     // Default light ray-march step size
	
	// Load Lab 4 shader (volume_emission) by default
	this->shader = Shader::Get("res/shaders/volume_emission.vs", "res/shaders/volume_emission.fs");
	
	// Load VDB file at startup
	loadVDB("res/volumes/bunny_cloud.vdb");
}

void VolumeScatteringMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	// Call parent setUniforms first (uploads all Lab 3 uniforms)
	VolumeMaterial::setUniforms(camera, model);
	
	// Task 3.1: Upload volume source selector
	this->shader->setUniform("u_volume_source", this->volume_source);
	
	// Task 3.1: Bind 3D texture if it exists
	if (this->texture) {
		this->texture->bind();
		this->shader->setUniform("u_volume_texture", 0);
	}
	
	// Task 3.2: Upload scattering parameters
	this->shader->setUniform("u_scattering_coeff", this->scattering_coeff);
	this->shader->setUniform("u_light_step_length", this->light_step_length);
	
	// Task 3.2: Upload light data (if light exists)
	if (Application::instance->light_list.size() > 0) {
		Light* light = Application::instance->light_list[0];
		light->setUniforms(this->shader, model);
	}
}

void VolumeScatteringMaterial::renderInMenu()
{
	// Call parent renderInMenu to show Lab 3 controls
	VolumeMaterial::renderInMenu();
	
	// Add Lab 4 controls
	ImGui::Separator();
	ImGui::Text("Lab 4: Volume Source (Task 3.1)");
	
	// Volume source combo box
	const char* volumeSources[] = { "VDB File", "3D Noise", "Constant" };
	ImGui::Combo("Volume Source", &this->volume_source, volumeSources, 3);
	
	// Show noise scale only when Noise is selected
	if (this->volume_source == 1) {
		ImGui::Text("(Noise scale controlled above)");
	}
	
	// Show info when Constant is selected
	if (this->volume_source == 2) {
		ImGui::Text("Using Absorption Coefficient as constant density");
	}
	
	// Task 3.2: Scattering controls (only in heterogeneous mode)
	if (this->is_homogeneous == 0) {
		ImGui::Separator();
		ImGui::Text("Lab 4: Scattering (Task 3.2)");
		ImGui::SliderFloat("Scattering Coeff", &this->scattering_coeff, 0.0f, 2.0f);
		ImGui::SliderFloat("Light Step Length", &this->light_step_length, 0.01f, 0.5f);
	}
}

void VolumeScatteringMaterial::loadVDB(std::string file_path)
{
	easyVDB::OpenVDBReader* vdbReader = new easyVDB::OpenVDBReader();
	vdbReader->read(file_path);

	// now, read the grid from the vdbReader and store the data in a 3D texture
	estimate3DTexture(vdbReader);
}

void VolumeScatteringMaterial::estimate3DTexture(easyVDB::OpenVDBReader* vdbReader)
{
	int resolution = 128;
	float radius = 2.0;

	int convertedGrids = 0;
	int convertedVoxels = 0;

	int totalGrids = vdbReader->gridsSize;
	int totalVoxels = totalGrids * pow(resolution, 3);

	float resolutionInv = 1.0f / resolution;
	int resolutionPow2 = pow(resolution, 2);
	int resolutionPow3 = pow(resolution, 3);

	// read all grids data and convert to texture
	for (unsigned int i = 0; i < totalGrids; i++) {
		easyVDB::Grid& grid = vdbReader->grids[i];
		float* data = new float[resolutionPow3];
		memset(data, 0, sizeof(float) * resolutionPow3);

		// Bbox
		easyVDB::Bbox bbox = easyVDB::Bbox();
		bbox = grid.getPreciseWorldBbox();
		glm::vec3 target = bbox.getCenter();
		glm::vec3 size = bbox.getSize();
		glm::vec3 step = size * resolutionInv;

		grid.transform->applyInverseTransformMap(step);
		target = target - (size * 0.5f);
		grid.transform->applyInverseTransformMap(target);
		target = target + (step * 0.5f);

		int x = 0;
		int y = 0;
		int z = 0;

		for (unsigned int j = 0; j < resolutionPow3; j++) {
			int baseX = x;
			int baseY = y;
			int baseZ = z;
			int baseIndex = baseX + baseY * resolution + baseZ * resolutionPow2;

			if (target.x >= 40 && target.y >= 40.33 && target.z >= 10.36) {
				int a = 0;
			}

			float value = grid.getValue(target);

			int cellBleed = radius;

			if (cellBleed) {
				for (int sx = -cellBleed; sx < cellBleed; sx++) {
					for (int sy = -cellBleed; sy < cellBleed; sy++) {
						for (int sz = -cellBleed; sz < cellBleed; sz++) {
							if (x + sx < 0.0 || x + sx >= resolution ||
								y + sy < 0.0 || y + sy >= resolution ||
								z + sz < 0.0 || z + sz >= resolution) {
								continue;
							}

							int targetIndex = baseIndex + sx + sy * resolution + sz * resolutionPow2;

							float offset = std::max(0.0, std::min(1.0, 1.0 - std::hypot(sx, sy, sz) / (radius / 2.0)));
							float dataValue = offset * value * 255.f;

							data[targetIndex] += dataValue;
							data[targetIndex] = std::min((float)data[targetIndex], 255.f);
						}
					}
				}
			}
			else {
				float dataValue = value * 255.f;

				data[baseIndex] += dataValue;
				data[baseIndex] = std::min((float)data[baseIndex], 255.f);
			}

			convertedVoxels++;

			if (z >= resolution) {
				break;
			}

			x++;
			target.x += step.x;

			if (x >= resolution) {
				x = 0;
				target.x -= step.x * resolution;

				y++;
				target.y += step.y;
			}

			if (y >= resolution) {
				y = 0;
				target.y -= step.y * resolution;

				z++;
				target.z += step.z;
			}

			// yield
		}

		// now we create the texture with the data
		// use this: https://www.khronos.org/opengl/wiki/OpenGL_Type
		// and this: https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage3D.xhtml
		this->texture = new Texture();
		this->texture->create3D(resolution, resolution, resolution, GL_RED, GL_FLOAT, false, data, GL_R8);
    }
}