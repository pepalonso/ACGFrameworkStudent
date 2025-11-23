#pragma once

#include "volumematerial.h"
#include "../../libraries/easyVDB/src/openvdbReader.h"
#include "../../libraries/easyVDB/src/grid.h"
#include "../../libraries/easyVDB/src/bbox.h"

class VolumeScatteringMaterial : public VolumeMaterial {
public:
	// Task 3.1: Volume source selection
	int volume_source;  // 0=VDB, 1=Noise, 2=Constant
	
	// Task 3.2: Scattering parameters
	float scattering_coeff;    // μs - How much light scatters
	float light_step_length;   // Step size for shadow ray-marching
	float phase_g;             // Henyey-Greenstein asymmetry parameter g (Task 3.3)

	// Constructor
	VolumeScatteringMaterial(float absorption_coeff = 0.5f);

	// VDB loading functions
	void loadVDB(std::string file_path);
	void estimate3DTexture(easyVDB::OpenVDBReader* vdbReader);

	// Overridden methods from VolumeMaterial
	void setUniforms(Camera* camera, glm::mat4 model) override;
	void renderInMenu() override;
};
