#include "nvpch.h"
#include "NavMeshDebugDrawer.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Rendering/Material.h>

#include <Volt/Rendering/DebugRenderer.h>

#include <glm/glm.hpp>

#include <DetourDebugDraw.h>

void NavMeshLine::Draw()
{
	Volt::DebugRenderer::DrawLine(start, end, color);
}

void NavMeshDebugDrawer::Shutdown()
{
	debugDrawCompiler = {};
}

void NavMeshDebugDrawer::DrawNavMesh()
{
	auto mesh = debugDrawCompiler.GetDebugMesh();
	if (mesh)
	{
		Volt::DebugRenderer::DrawMesh(mesh, glm::mat4(1.f));

		for (auto line : debugDrawCompiler.GetDebugLines())
		{
			line.Draw();
		}
	}

	// OLD

	//const auto path = Volt::AssetManager::GetRelativePath(DebugMaterialPath);
	//auto& mesh = navmesh->GetMesh();
	//mesh->SetMaterial(Volt::AssetManager::GetAsset<Volt::Material>(path));

	//Volt::DebugRenderer::DrawMesh(mesh, glm::mat4(1.f));

	//// Draw NavMesh Lines

	//auto vertices = mesh->GetVertices();
	//auto indices = mesh->GetIndices();
	//
	//for (uint32_t i = 0; i < indices.size(); i += 3)
	//{
	//	Volt::DebugRenderer::DrawLine(vertices[indices[i]].position, vertices[indices[i + 1]].position, glm::vec4(0.2f, 0.2, 0.2, 1.f));
	//	Volt::DebugRenderer::DrawLine(vertices[indices[i + 1]].position, vertices[indices[i + 2]].position, glm::vec4(0.2, 0.2, 0.2, 1.f));
	//	Volt::DebugRenderer::DrawLine(vertices[indices[i + 2]].position, vertices[indices[i]].position, glm::vec4(0.2, 0.2, 0.2, 1.f));
	//}
}

void NavMeshDebugDrawer::DrawLinks(const Vector<Volt::AI::NavLinkConnection>& links)
{
	for (const auto& link : links)
	{
		Volt::DebugRenderer::DrawLineSphere(link.start, 100.f, glm::vec4(1.f, 1.f, 0.f, 1.f));
		Volt::DebugRenderer::DrawLine(link.start, link.end, glm::vec4(1.f, 1.f, 0.f, 1.f));
		Volt::DebugRenderer::DrawLineSphere(link.end, 100.f, glm::vec4(1.f, 1.f, 0.f, 1.f));
	}
}

void NavMeshDebugDrawer::DrawPath(const Vector<glm::vec3>& path)
{
	for (uint32_t i = 0; i < path.size(); i++)
	{
		if (i == path.size() - 1)
		{
			Volt::DebugRenderer::DrawLine(path[i], path[0], glm::vec4(1.f, 0.f, 0.f, 1.f));
		}
		else
		{
			Volt::DebugRenderer::DrawLine(path[i], path[i + 1], glm::vec4(1.f, 0.f, 0.f, 1.f));
		}
	}
}

void NavMeshDebugDrawer::CompileDebugMesh(Volt::AI::NavMesh* navmesh)
{
	if (navmesh)
	{
		debugDrawCompiler.Clear();
		duDebugDrawNavMeshWithClosedList(&debugDrawCompiler, *navmesh->GetNavMesh()->GetNavMesh().get(), *navmesh->GetNavMesh()->GetNavMeshQuery().get(), 0);
	}
}

void NavMeshDrawCompiler::Clear()
{
	myVertices.clear();
	mySubmeshes.clear();
	myIndices.clear();
	myLineVertices.clear();
}

void NavMeshDrawCompiler::begin(duDebugDrawPrimitives prim, float size)
{
	myPrimitive = prim;
	if(myPrimitive == DU_DRAW_TRIS)
	{
		Volt::SubMesh submesh{};
		submesh.vertexStartOffset = static_cast<uint32_t>(myVertices.size());
		submesh.indexStartOffset = static_cast<uint32_t>(myIndices.size());
		submesh.materialIndex = 0;

		mySubmeshes.emplace_back(submesh);
	}
}

void NavMeshDrawCompiler::vertex(const float* pos, unsigned int color)
{
	Volt::Vertex vertex;
	vertex.position = *(glm::vec3*)pos;

	if (myPrimitive == DU_DRAW_TRIS)
	{
		const uint32_t index = static_cast<uint32_t>(myVertices.size());
		myIndices.push_back(index);
		myVertices.emplace_back(vertex);
	}
	else if (myPrimitive == DU_DRAW_LINES)
	{
		myLineVertices.emplace_back(vertex);
	}
}

void NavMeshDrawCompiler::vertex(const float x, const float y, const float z, unsigned int color)
{
	Volt::Vertex vertex;
	vertex.position = glm::vec3(x, y, z);

	if (myPrimitive == DU_DRAW_TRIS)
	{
		const uint32_t index = static_cast<uint32_t>(myVertices.size());
		myIndices.push_back(index);
		myVertices.emplace_back(vertex);
	}
	else if (myPrimitive == DU_DRAW_LINES)
	{
		myLineVertices.emplace_back(vertex);
	}
}

void NavMeshDrawCompiler::vertex(const float* pos, unsigned int color, const float* uv)
{
	Volt::Vertex vertex;
	vertex.position = *(glm::vec3*)pos;
	vertex.uv = *(glm::vec2*)uv;

	if (myPrimitive == DU_DRAW_TRIS)
	{
		const uint32_t index = static_cast<uint32_t>(myVertices.size());
		myIndices.push_back(index);
		myVertices.emplace_back(vertex);
	}
	else if (myPrimitive == DU_DRAW_LINES)
	{
		myLineVertices.emplace_back(vertex);
	}
}

void NavMeshDrawCompiler::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
	Volt::Vertex vertex;
	vertex.position = glm::vec3(x, y, z);
	vertex.uv = glm::vec2(u, v);

	if (myPrimitive == DU_DRAW_TRIS)
	{
		const uint32_t index = static_cast<uint32_t>(myVertices.size());
		myIndices.push_back(index);
		myVertices.emplace_back(vertex);
	}
	else if (myPrimitive == DU_DRAW_LINES)
	{
		myLineVertices.emplace_back(vertex);
	}
}

void NavMeshDrawCompiler::end()
{
	if (myPrimitive == DU_DRAW_TRIS)
	{
		VT_ASSERT_MSG(!mySubmeshes.empty(), "Missing NavMeshDrawCompiler::begin call");

		mySubmeshes.back().vertexCount = static_cast<uint32_t>(myIndices.size()) - mySubmeshes.back().vertexStartOffset;
		mySubmeshes.back().indexCount = static_cast<uint32_t>(myVertices.size()) - mySubmeshes.back().indexStartOffset;
	}
}

Ref<Volt::Mesh> NavMeshDrawCompiler::GetDebugMesh() const
{
	if (myVertices.empty() || myIndices.empty()) { return nullptr; }

	const auto path = Volt::AssetManager::GetRelativePath(NavMeshDebugDrawer::DebugMaterialPath);
	auto material = Volt::AssetManager::GetAsset<Volt::Material>(path);

	Volt::MaterialTable materialTable{};
	materialTable.SetMaterial(material->handle, 0);

	return CreateRef<Volt::Mesh>(myVertices, myIndices, materialTable, mySubmeshes);
}

Vector<NavMeshLine> NavMeshDrawCompiler::GetDebugLines() const
{
	Vector<NavMeshLine> lines;

	for (uint32_t i = 0; i < myLineVertices.size(); i += 2)
	{
		NavMeshLine line;
		line.start = myLineVertices[i].position;
		line.end = myLineVertices[i + 1].position;
		line.color = glm::vec4(0.2, 0.2, 0.2, 1.f);
		lines.emplace_back(line);
	}

	return lines;
}
