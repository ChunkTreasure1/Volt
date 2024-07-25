#pragma once

#include "Volt/Rendering/Resources/BindlessResource.h"

#include <VoltRHI/Images/Image3D.h>

#include <CoreUtilities/Containers/Vector.h>

namespace Volt
{
	class Mesh;
	struct SubMesh;

	struct MeshSDF
	{
		glm::uvec3 size;

		glm::vec3 min;
		glm::vec3 max;

		BindlessResourceRef<RHI::Image3D> sdfTexture;
		Vector<float> sdf;
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
