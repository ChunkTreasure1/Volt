#pragma once

#include "Volt/Rendering/RendererCommon.h"
#include "Volt/Rendering/SceneRenderer.h"

namespace Volt
{
	class Texture2D;
	class Mesh;
	class Material;
	class DebugRenderer
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Execute(SceneRenderer::PerThreadData& submitCommands);

		static void DrawBillboard(const glm::vec3& pos, const glm::vec3& size, const glm::vec4& color, uint32_t id = 0);
		static void DrawBillboard(Ref<Texture2D> texture, const glm::vec3& pos, const glm::vec3& size, const glm::vec4& color, uint32_t id = 0);

		static void DrawLine(const glm::vec3& startPosition, const glm::vec3& endPosition, const glm::vec4& color = { 1.f });
		static void DrawSprite(const glm::vec3& position, const glm::vec3& rotation = { 0.f }, const glm::vec3& scale = { 1.f }, const glm::vec4& color = { 1.f });
		static void DrawText(const std::string& text, const glm::vec3& position, const glm::vec3& rotation = { 0.f }, const glm::vec3& scale = { 1.f });

		static void DrawMesh(Ref<Mesh> mesh, const glm::vec3& position, const glm::vec3& rotation = { 0.f }, const glm::vec3& scale = { 1.f });
		static void DrawMesh(Ref<Mesh> mesh, const glm::mat4& transform);
		static void DrawMesh(Ref<Mesh> mesh, Ref<Material> material, const glm::mat4& transform, uint32_t id = 0);

		static void DrawLineSphere(const glm::vec3& center, float radius, const glm::vec4& color = { 1.f });
		static void DrawLineBox(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color = { 1.f });
		static void DrawLineOBB(const glm::vec3& min, const glm::vec3& max, const glm::mat4& transform, const glm::vec4& color = { 1.f });

		static void DrawSphere(const glm::vec3& position, const glm::vec3& scale = { 1.f }, const glm::vec4& color = { 1.f });
		static void DrawCube(const glm::vec3& position, const glm::vec3& rotation = { 0.f }, const glm::vec3& scale = { 1.f }, const glm::vec4& color = { 1.f });

	private:
		DebugRenderer() = delete;
	};
}
