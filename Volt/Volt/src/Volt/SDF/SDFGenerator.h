#pragma once

#include "Volt/Rendering/BindlessResource.h"

#include <RHIModule/Images/Image3D.h>

#include <CoreUtilities/Containers/Vector.h>
#include <CoreUtilities/Containers/SparseBrickMap.h>

namespace Volt
{
	class Mesh;
	struct SubMesh;

	constexpr uint32_t BRICK_SIZE = 8;

	struct SDFBrick
	{
		glm::vec3 min;
		glm::vec3 max;

		float data[BRICK_SIZE * BRICK_SIZE * BRICK_SIZE];
		bool hasData = false;
	};

	struct MeshSDF
	{
		glm::uvec3 size;

		glm::vec3 min;
		glm::vec3 max;

		BindlessResourceRef<RHI::Image3D> sdfTexture;
		RefPtr<RHI::StorageBuffer> sdfBricksBuffer;
		Vector<SDFBrick> brickGrid;
	};

	class SDFGenerator
	{
	public:
		SDFGenerator();

		Vector<MeshSDF> Generate(Mesh& mesh);

	private:
		MeshSDF GenerateForSubMesh(Mesh& mesh, const uint32_t index, const SubMesh& subMesh);
	};
}
