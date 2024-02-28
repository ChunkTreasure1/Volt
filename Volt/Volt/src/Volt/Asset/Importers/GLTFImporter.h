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
			std::vector<Primitive> primitives;
		};

		struct Node
		{
			Node* parent;
			std::vector<Node> children;
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
		void ImportMeshImpl(const std::filesystem::path& path, Ref<Mesh>& mesh) override;
		void ImportSkeletonImpl(const std::filesystem::path&, Ref<Skeleton>& skeleton) override { }
		void ImportAnimationImpl(const std::filesystem::path&, Ref<Skeleton>, Ref<Animation>&) override { }

	private:
		void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& inputModel, GLTF::Node* parent, Ref<Mesh> outMesh);
	};
}
