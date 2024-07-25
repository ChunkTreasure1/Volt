#pragma once

#include <CoreUtilities/Containers/Vector.h>

namespace Volt
{
	class Mesh;
	struct SubMesh;

	struct MeshSDF
	{
		glm::uvec3 size;
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
