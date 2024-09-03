#pragma once

#include "MeshTypeImporter.h"
#include "Volt/Rendering/Vertex.h"

#include <glm/glm.hpp>

namespace tinygltf
{
	class Node;
	class Model;
}

namespace Volt
{
	namespace GLTF
	{
		struct Node;
		struct Primitive
		{
			uint32_t firstIndex;
			uint32_t indexCount;
			int32_t materialIndex;
		};

		struct Mesh
		{
			Vector<Primitive> primitives;
		};

		struct Node
		{
			Node* parent;
			Vector<Node> children;
			Mesh mesh;
			glm::mat4 transform = { 1.f };
		};
	}

	class Mesh;
	class GLTFImporter : public MeshTypeImporter
	{
	public:
		GLTFImporter() = default;

	protected:
		bool ImportMeshImpl(const std::filesystem::path& path, Mesh& dstMesh) override;
		bool ImportSkeletonImpl(const std::filesystem::path&, Skeleton& dstSkeleton) override { return false; }
		bool ImportAnimationImpl(const std::filesystem::path&, Ref<Skeleton>, Animation& dstAnimation) override { return false; }

	private:
		void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& inputModel, GLTF::Node* parent, Mesh& outMesh);
	};
}
