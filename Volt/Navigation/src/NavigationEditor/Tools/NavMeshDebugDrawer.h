#pragma once

#include <Navigation/Core/NavigationSystem.h>

#include <DebugDraw.h>

struct NavMeshLine
{
	void Draw();

	glm::vec3 start;
	glm::vec3 end;
	glm::vec4 color;
};

class NavMeshDrawCompiler : public duDebugDraw
{
public:
	void Clear();

	virtual void depthMask(bool state) {};
	virtual void texture(bool state) {};
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
	virtual void vertex(const float* pos, unsigned int color);
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	virtual void vertex(const float* pos, unsigned int color, const float* uv);
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);
	virtual void end();

	Ref<Volt::Mesh> GetDebugMesh() const;
	Vector<NavMeshLine> GetDebugLines() const;

private:
	duDebugDrawPrimitives myPrimitive;

	Vector<Volt::Vertex> myLineVertices;

	Vector<Volt::Vertex> myVertices;
	Vector<Volt::SubMesh> mySubmeshes;
	Vector<uint32_t> myIndices;
};

class NavMeshDebugDrawer
{
public:
	NavMeshDebugDrawer() = delete;
	~NavMeshDebugDrawer() = delete;

	static void Shutdown();

	static void DrawNavMesh();
	static void DrawLinks(const Vector<Volt::AI::NavLinkConnection>& links);
	static void DrawPath(const Vector<glm::vec3>& path);

	static void CompileDebugMesh(Volt::AI::NavMesh* navmesh);

	inline static const std::filesystem::path DebugMaterialPath = "Engine\\Materials\\M_NavMesh.vtmat";

private:
	inline static NavMeshDrawCompiler debugDrawCompiler;
};
