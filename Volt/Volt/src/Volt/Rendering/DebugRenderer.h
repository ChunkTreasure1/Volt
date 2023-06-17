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

		static void DrawBillboard(const gem::vec3& pos, const gem::vec3& size, const gem::vec4& color, uint32_t id = 0);
		static void DrawBillboard(Ref<Texture2D> texture, const gem::vec3& pos, const gem::vec3& size, const gem::vec4& color, uint32_t id = 0);

		static void DrawLine(const gem::vec3& startPosition, const gem::vec3& endPosition, const gem::vec4& color = { 1.f });
		static void DrawSprite(const gem::vec3& position, const gem::vec3& rotation = { 0.f }, const gem::vec3& scale = { 1.f }, const gem::vec4& color = { 1.f });
		static void DrawText(const std::string& text, const gem::vec3& position, const gem::vec3& rotation = { 0.f }, const gem::vec3& scale = { 1.f });

		static void DrawMesh(Ref<Mesh> mesh, const gem::vec3& position, const gem::vec3& rotation = { 0.f }, const gem::vec3& scale = { 1.f });
		static void DrawMesh(Ref<Mesh> mesh, const gem::mat4& transform);
		static void DrawMesh(Ref<Mesh> mesh, Ref<Material> material, const gem::mat4& transform, uint32_t id = 0);

		static void DrawLineSphere(const gem::vec3& center, float radius, const gem::vec4& color = { 1.f });
		static void DrawLineBox(const gem::vec3& min, const gem::vec3& max, const gem::vec4& color = { 1.f });
		static void DrawLineOBB(const gem::vec3& min, const gem::vec3& max, const gem::mat4& transform, const gem::vec4& color = { 1.f });

		static void DrawSphere(const gem::vec3& position, const gem::vec3& scale = { 1.f }, const gem::vec4& color = { 1.f });
		static void DrawCube(const gem::vec3& position, const gem::vec3& rotation = { 0.f }, const gem::vec3& scale = { 1.f }, const gem::vec4& color = { 1.f });

	private:
		DebugRenderer() = delete;
	};
}
