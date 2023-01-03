#include "vtpch.h"
#include "Shape.h"

#include "Volt/Rendering/Vertex.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/AssetManager.h"

namespace Volt
{
	Ref<Mesh> Shape::CreateUnitCube()
	{
		std::vector<Vertex> vertecies =
		{
			{{  50,  50,  50 }},
			{{  50,  50, -50 }},
			{{  50, -50,  50 }},
			{{ -50,  50,  50 }},

			{{  50, -50, -50 }},
			{{ -50,  50, -50 }},
			{{ -50, -50,  50 }},
			{{ -50, -50, -50 }}

		};

		std::vector<uint32_t> indecies =
		{
			//side right
			0,2,1,
			2,4,1,

			//side front
			1,4,7,
			7,5,1,

			//side left
			5,7,6,
			6,3,5,

			//side back
			3,6,0,
			6,2,0,

			//top
			3,0,5,
			0,1,5,

			//bottom
			6,4,2,
			6,7,4
		};

		Ref<Material> material = AssetManager::GetAsset<Material>("Assets/Meshes/Primitives/SM_Cube.vtmat");
		Ref<Mesh> mesh = CreateRef<Mesh>(vertecies, indecies, material); 

		return mesh;
	}
}