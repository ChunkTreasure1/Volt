#include "vtpch.h"
#include "DebugRenderer.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"

namespace Volt
{
	struct DebugRenderData
	{
		std::vector<std::function<void(SceneRenderer::PerThreadData&)>> functionQueue;
	};

	Scope<DebugRenderData> s_debugRenderData;

	void DebugRenderer::Initialize()
	{
		s_debugRenderData = CreateScope<DebugRenderData>();
	}

	void DebugRenderer::Shutdown()
	{
		s_debugRenderData = nullptr;
	}

	void DebugRenderer::Execute(SceneRenderer::PerThreadData& submitCommands)
	{
		for (const auto& f : s_debugRenderData->functionQueue)
		{
			f(submitCommands);
		}

		s_debugRenderData->functionQueue.clear();
	}

	void DebugRenderer::DrawBillboard(const gem::vec3& pos, const gem::vec3& size, const gem::vec4& color, uint32_t id)
	{
		s_debugRenderData->functionQueue.emplace_back([pos, size, color, id](SceneRenderer::PerThreadData& threadData)
		{
			auto& cmd = threadData.billboardCommands.emplace_back();
			cmd.position = pos;
			cmd.scale = size;
			cmd.color = color;
			cmd.id = id;
		});
	}

	void DebugRenderer::DrawBillboard(Ref<Texture2D> texture, const gem::vec3& pos, const gem::vec3& size, const gem::vec4& color, uint32_t id)
	{
		s_debugRenderData->functionQueue.emplace_back([pos, size, color, id, texture](SceneRenderer::PerThreadData& threadData)
		{
			auto& cmd = threadData.billboardCommands.emplace_back();
			cmd.position = pos;
			cmd.scale = size;
			cmd.color = color;
			cmd.id = id;
			cmd.texture = texture;
		});
	}

	void DebugRenderer::DrawLine(const gem::vec3& startPosition, const gem::vec3& endPosition, const gem::vec4& color)
	{
		s_debugRenderData->functionQueue.emplace_back([startPosition, endPosition, color](SceneRenderer::PerThreadData& data)
		{
			auto& cmd = data.lineCommands.emplace_back();
			cmd.startPosition = startPosition;
			cmd.endPosition = endPosition;
			cmd.color = color;
		});
	}

	void DebugRenderer::DrawSprite(const gem::vec3& position, const gem::vec3& rotation, const gem::vec3& scale, const gem::vec4& color)
	{
		//s_debugRenderData->functionQueue.emplace_back([position, rotation, scale, color]()
		//	{
		//		const gem::mat4 transform = gem::translate(gem::mat4{ 1.f }, position) * gem::mat4_cast(gem::quat(rotation)) * gem::scale(gem::mat4{ 1.f }, scale);
		//		Renderer::SubmitSprite(transform, color);
		//	});
	}

	void DebugRenderer::DrawText(const std::string& text, const gem::vec3& position, const gem::vec3& rotation, const gem::vec3& scale)
	{
		//s_debugRenderData->functionQueue.emplace_back([text, position, rotation, scale]()
		//	{
		//		const gem::mat4 transform = gem::translate(gem::mat4{ 1.f }, position) * gem::mat4_cast(gem::quat(rotation)) * gem::scale(gem::mat4{ 1.f }, scale);
		//		//Renderer::SubmitText(text, Renderer::GetDefaultData().de)
		//	});
	}

	void DebugRenderer::DrawMesh(Ref<Mesh> mesh, const gem::vec3& position, const gem::vec3& rotation, const gem::vec3& scale)
	{
		s_debugRenderData->functionQueue.emplace_back([mesh, position, rotation, scale](SceneRenderer::PerThreadData& data)
		{
			const gem::mat4 transform = gem::translate(gem::mat4{ 1.f }, position)* gem::mat4_cast(gem::quat(rotation))* gem::scale(gem::mat4{ 1.f }, scale);

			for (uint32_t i = 0; const auto & subMesh : mesh->GetSubMeshes())
			{
				auto& cmd = data.submitCommands.emplace_back();
				cmd.mesh = mesh;
				cmd.material = mesh->GetMaterial()->TryGetSubMaterialAt(subMesh.materialIndex);
				cmd.subMesh = subMesh;
				cmd.transform = transform * subMesh.transform;
				cmd.subMeshIndex = i;
			}
		});
	}

	void DebugRenderer::DrawMesh(Ref<Mesh> mesh, const gem::mat4& transform)
	{
		s_debugRenderData->functionQueue.emplace_back([mesh, transform](SceneRenderer::PerThreadData& data)
		{
			for (uint32_t i = 0; const auto & subMesh : mesh->GetSubMeshes())
			{
				auto& cmd = data.submitCommands.emplace_back();
				cmd.mesh = mesh;
				cmd.material = mesh->GetMaterial()->TryGetSubMaterialAt(subMesh.materialIndex);
				cmd.subMesh = subMesh;
				cmd.transform = transform * subMesh.transform;
				cmd.subMeshIndex = i;
			}
		});
	}

	void DebugRenderer::DrawMesh(Ref<Mesh> mesh, Ref<Material> material, const gem::mat4& transform, uint32_t id)
	{
		s_debugRenderData->functionQueue.emplace_back([mesh, transform, id, material](SceneRenderer::PerThreadData& data)
		{
			for (uint32_t i = 0; const auto & subMesh : mesh->GetSubMeshes())
			{
				auto& cmd = data.submitCommands.emplace_back();
				cmd.mesh = mesh;
				cmd.material = material->TryGetSubMaterialAt(subMesh.materialIndex);
				cmd.subMesh = subMesh;
				cmd.transform = transform * subMesh.transform;
				cmd.subMeshIndex = i;
				cmd.id = id;
			}
		});
	}

	void DebugRenderer::DrawLineSphere(const gem::vec3& center, float radius, const gem::vec4& color)
	{
		std::vector<gem::vec3> positions;
		positions.reserve(36);

		for (float i = 0; i <= 360; i += 10.f)
		{
			const float x = radius * gem::cos(gem::radians(i));
			const float z = radius * gem::sin(gem::radians(i));

			positions.emplace_back(x, 0.f, z);
		}

		s_debugRenderData->functionQueue.emplace_back([positions, center, color](SceneRenderer::PerThreadData& data)
		{
			for (size_t i = 1; i < positions.size(); i++)
			{
				auto& cmd = data.lineCommands.emplace_back();
				cmd.startPosition = center + positions.at(i - 1);
				cmd.endPosition = center + positions.at(i);
				cmd.color = color;
			}

			auto& cmd = data.lineCommands.emplace_back();
			cmd.startPosition = center + positions.back();
			cmd.endPosition = center + positions.front();
			cmd.color = color;
		});

		positions.clear();

		for (float i = 0; i <= 360; i += 10.f)
		{
			const float x = radius * gem::cos(gem::radians(i));
			const float z = radius * gem::sin(gem::radians(i));

			positions.emplace_back(x, z, 0.f);
		}

		s_debugRenderData->functionQueue.emplace_back([positions, center, color](SceneRenderer::PerThreadData& data)
		{
			for (size_t i = 1; i < positions.size(); i++)
			{
				auto& cmd = data.lineCommands.emplace_back();
				cmd.startPosition = center + positions.at(i - 1);
				cmd.endPosition = center + positions.at(i);
				cmd.color = color;
			}

			auto& cmd = data.lineCommands.emplace_back();
			cmd.startPosition = center + positions.back();
			cmd.endPosition = center + positions.front();
			cmd.color = color;
		});

		positions.clear();

		for (float i = 0; i <= 360; i += 10.f)
		{
			const float x = radius * gem::cos(gem::radians(i));
			const float z = radius * gem::sin(gem::radians(i));

			positions.emplace_back(0.f, z, x);
		}

		s_debugRenderData->functionQueue.emplace_back([positions, center, color](SceneRenderer::PerThreadData& data)
		{
			for (size_t i = 1; i < positions.size(); i++)
			{
				auto& cmd = data.lineCommands.emplace_back();
				cmd.startPosition = center + positions.at(i - 1);
				cmd.endPosition = center + positions.at(i);
				cmd.color = color;
			}

			auto& cmd = data.lineCommands.emplace_back();
			cmd.startPosition = center + positions.back();
			cmd.endPosition = center + positions.front();
			cmd.color = color;
		});
	}

	void DebugRenderer::DrawLineBox(const gem::vec3& min, const gem::vec3& max, const gem::vec4& color)
	{
		gem::vec3 p1 = { min };
		gem::vec3 p2 = { min.x, min.y, max.z };
		gem::vec3 p3 = { min.x, max.y, min.z };
		gem::vec3 p4 = { min.x, max.y, max.z };

		gem::vec3 p5 = { max.x, min.y, min.z };
		gem::vec3 p6 = { max.x, min.y, max.z };
		gem::vec3 p7 = { max.x, max.y, min.z };
		gem::vec3 p8 = { max };

		s_debugRenderData->functionQueue.emplace_back([=](SceneRenderer::PerThreadData& data)
		{
			// Bottom
			data.lineCommands.emplace_back(p1, p2, color);
			data.lineCommands.emplace_back(p2, p4, color);
			data.lineCommands.emplace_back(p4, p3, color);
			data.lineCommands.emplace_back(p3, p1, color);

			// Top
			data.lineCommands.emplace_back(p5, p6, color);
			data.lineCommands.emplace_back(p6, p8, color);
			data.lineCommands.emplace_back(p8, p7, color);
			data.lineCommands.emplace_back(p7, p5, color);

			// Connecting
			data.lineCommands.emplace_back(p1, p5, color);
			data.lineCommands.emplace_back(p2, p6, color);
			data.lineCommands.emplace_back(p3, p7, color);
			data.lineCommands.emplace_back(p4, p8, color);

		});
	}

	void DebugRenderer::DrawLineOBB(const gem::vec3& min, const gem::vec3& max, const gem::mat4& transform, const gem::vec4& color)
	{
		gem::vec3 p1 = transform * gem::vec4{ min.x, min.y, min.z, 1.f };
		gem::vec3 p2 = transform * gem::vec4{ min.x, min.y, max.z, 1.f };
		gem::vec3 p3 = transform * gem::vec4{ min.x, max.y, min.z, 1.f };
		gem::vec3 p4 = transform * gem::vec4{ min.x, max.y, max.z, 1.f };

		gem::vec3 p5 = transform * gem::vec4{ max.x, min.y, min.z, 1.f };
		gem::vec3 p6 = transform * gem::vec4{ max.x, min.y, max.z, 1.f };
		gem::vec3 p7 = transform * gem::vec4{ max.x, max.y, min.z, 1.f };
		gem::vec3 p8 = transform * gem::vec4{ max.x, max.y, max.z, 1.f };

		s_debugRenderData->functionQueue.emplace_back([=](SceneRenderer::PerThreadData& data)
		{
			// Bottom
			data.lineCommands.emplace_back(p1, p2, color);
			data.lineCommands.emplace_back(p2, p4, color);
			data.lineCommands.emplace_back(p4, p3, color);
			data.lineCommands.emplace_back(p3, p1, color);

			//// Top
			data.lineCommands.emplace_back(p5, p6, color);
			data.lineCommands.emplace_back(p6, p8, color);
			data.lineCommands.emplace_back(p8, p7, color);
			data.lineCommands.emplace_back(p7, p5, color);

			//// Connecting
			data.lineCommands.emplace_back(p1, p5, color);
			data.lineCommands.emplace_back(p2, p6, color);
			data.lineCommands.emplace_back(p3, p7, color);
			data.lineCommands.emplace_back(p4, p8, color);

		});
	}

	void DebugRenderer::DrawSphere(const gem::vec3& position, const gem::vec3& scale, const gem::vec4& color)
	{
	}

	void DebugRenderer::DrawCube(const gem::vec3& position, const gem::vec3& rotation, const gem::vec3& scale, const gem::vec4& color)
	{
	}
}
