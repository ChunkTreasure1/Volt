#include "vtpch.h"
#include "DebugRenderer.h"

#include "Volt/Asset/Mesh/Mesh.h"

namespace Volt
{
	struct DebugRenderData
	{
		//Vector<std::function<void(SceneRenderer::PerThreadData&)>> functionQueue;
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

	//void DebugRenderer::Execute(SceneRenderer::PerThreadData& submitCommands)
	//{
	//	for (const auto& f : s_debugRenderData->functionQueue)
	//	{
	//		f(submitCommands);
	//	}

	//	s_debugRenderData->functionQueue.clear();
	//}

	void DebugRenderer::DrawBillboard(const glm::vec3& pos, const glm::vec3& size, const glm::vec4& color, uint32_t id)
	{
		//s_debugRenderData->functionQueue.emplace_back([pos, size, color, id](SceneRenderer::PerThreadData& threadData)
		//{
		//	auto& cmd = threadData.billboardCommands.emplace_back();
		//	cmd.position = pos;
		//	cmd.scale = size;
		//	cmd.color = color;
		//	cmd.id = id;
		//});
	}

	void DebugRenderer::DrawBillboard(Ref<Texture2D> texture, const glm::vec3& pos, const glm::vec3& size, const glm::vec4& color, uint32_t id)
	{
		//s_debugRenderData->functionQueue.emplace_back([pos, size, color, id, texture](SceneRenderer::PerThreadData& threadData)
		//{
		//	auto& cmd = threadData.billboardCommands.emplace_back();
		//	cmd.position = pos;
		//	cmd.scale = size;
		//	cmd.color = color;
		//	cmd.id = id;
		//	cmd.texture = texture;
		//});
	}

	void DebugRenderer::DrawLine(const glm::vec3& startPosition, const glm::vec3& endPosition, const glm::vec4& color)
	{
		//s_debugRenderData->functionQueue.emplace_back([startPosition, endPosition, color](SceneRenderer::PerThreadData& data)
		//{
		//	auto& cmd = data.lineCommands.emplace_back();
		//	cmd.startPosition = startPosition;
		//	cmd.endPosition = endPosition;
		//	cmd.color = color;
		//});
	}

	void DebugRenderer::DrawSprite(const glm::vec3&, const glm::vec3&, const glm::vec3&, const glm::vec4&)
	{
		//s_debugRenderData->functionQueue.emplace_back([position, rotation, scale, color]()
		//	{
		//		const glm::mat4 transform = glm::translate(glm::mat4{ 1.f }, position) * glm::mat4_cast(glm::quat(rotation)) * glm::scale(glm::mat4{ 1.f }, scale);
		//		Renderer::SubmitSprite(transform, color);
		//	});
	}

	void DebugRenderer::DrawText(const std::string&, const glm::vec3& , const glm::vec3&, const glm::vec3&)
	{
		//s_debugRenderData->functionQueue.emplace_back([text, position, rotation, scale]()
		//	{
		//		const glm::mat4 transform = glm::translate(glm::mat4{ 1.f }, position) * glm::mat4_cast(glm::quat(rotation)) * glm::scale(glm::mat4{ 1.f }, scale);
		//		//Renderer::SubmitText(text, Renderer::GetDefaultData().de)
		//	});
	}

	void DebugRenderer::DrawMesh(Ref<Mesh> mesh, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
	{
		//s_debugRenderData->functionQueue.emplace_back([mesh, position, rotation, scale](SceneRenderer::PerThreadData& data)
		//{
		//	const glm::mat4 transform = glm::translate(glm::mat4{ 1.f }, position)* glm::mat4_cast(glm::quat(rotation))* glm::scale(glm::mat4{ 1.f }, scale);

		//	for (uint32_t i = 0; const auto & subMesh : mesh->GetSubMeshes())
		//	{
		//		auto& cmd = data.submitCommands.emplace_back();
		//		cmd.mesh = mesh;
		//		cmd.material = mesh->GetMaterial()->TryGetSubMaterialAt(subMesh.materialIndex);
		//		cmd.subMesh = subMesh;
		//		cmd.transform = transform * subMesh.transform;
		//		cmd.subMeshIndex = i;
		//	}
		//});
	}

	void DebugRenderer::DrawMesh(Ref<Mesh> mesh, const glm::mat4& transform)
	{
		//s_debugRenderData->functionQueue.emplace_back([mesh, transform](SceneRenderer::PerThreadData& data)
		//{
		//	for (uint32_t i = 0; const auto & subMesh : mesh->GetSubMeshes())
		//	{
		//		auto& cmd = data.submitCommands.emplace_back();
		//		cmd.mesh = mesh;
		//		cmd.material = mesh->GetMaterial()->TryGetSubMaterialAt(subMesh.materialIndex);
		//		cmd.subMesh = subMesh;
		//		cmd.transform = transform * subMesh.transform;
		//		cmd.subMeshIndex = i;
		//	}
		//});
	}

	void DebugRenderer::DrawMesh(Ref<Mesh> mesh, Ref<Material> material, const glm::mat4& transform, uint32_t id)
	{
		//s_debugRenderData->functionQueue.emplace_back([mesh, transform, id, material](SceneRenderer::PerThreadData& data)
		//{
		//	for (uint32_t i = 0; const auto & subMesh : mesh->GetSubMeshes())
		//	{
		//		auto& cmd = data.submitCommands.emplace_back();
		//		cmd.mesh = mesh;
		//		cmd.material = material->TryGetSubMaterialAt(subMesh.materialIndex);
		//		cmd.subMesh = subMesh;
		//		cmd.transform = transform * subMesh.transform;
		//		cmd.subMeshIndex = i;
		//		cmd.id = id;
		//	}
		//});
	}

	void DebugRenderer::DrawLineSphere(const glm::vec3& center, float radius, const glm::vec4& color)
	{
		/*Vector<glm::vec3> positions;
		positions.reserve(36);

		for (float i = 0; i <= 360; i += 10.f)
		{
			const float x = radius * glm::cos(glm::radians(i));
			const float z = radius * glm::sin(glm::radians(i));

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
			const float x = radius * glm::cos(glm::radians(i));
			const float z = radius * glm::sin(glm::radians(i));

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
			const float x = radius * glm::cos(glm::radians(i));
			const float z = radius * glm::sin(glm::radians(i));

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
		});*/
	}

	void DebugRenderer::DrawLineBox(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color)
	{
		//glm::vec3 p1 = { min };
		//glm::vec3 p2 = { min.x, min.y, max.z };
		//glm::vec3 p3 = { min.x, max.y, min.z };
		//glm::vec3 p4 = { min.x, max.y, max.z };

		//glm::vec3 p5 = { max.x, min.y, min.z };
		//glm::vec3 p6 = { max.x, min.y, max.z };
		//glm::vec3 p7 = { max.x, max.y, min.z };
		//glm::vec3 p8 = { max };

		//s_debugRenderData->functionQueue.emplace_back([=](SceneRenderer::PerThreadData& data)
		//{
		//	// Bottom
		//	data.lineCommands.emplace_back(p1, p2, color);
		//	data.lineCommands.emplace_back(p2, p4, color);
		//	data.lineCommands.emplace_back(p4, p3, color);
		//	data.lineCommands.emplace_back(p3, p1, color);

		//	// Top
		//	data.lineCommands.emplace_back(p5, p6, color);
		//	data.lineCommands.emplace_back(p6, p8, color);
		//	data.lineCommands.emplace_back(p8, p7, color);
		//	data.lineCommands.emplace_back(p7, p5, color);

		//	// Connecting
		//	data.lineCommands.emplace_back(p1, p5, color);
		//	data.lineCommands.emplace_back(p2, p6, color);
		//	data.lineCommands.emplace_back(p3, p7, color);
		//	data.lineCommands.emplace_back(p4, p8, color);

		//});
	}

	void DebugRenderer::DrawLineOBB(const glm::vec3& min, const glm::vec3& max, const glm::mat4& transform, const glm::vec4& color)
	{
		//glm::vec3 p1 = transform * glm::vec4{ min.x, min.y, min.z, 1.f };
		//glm::vec3 p2 = transform * glm::vec4{ min.x, min.y, max.z, 1.f };
		//glm::vec3 p3 = transform * glm::vec4{ min.x, max.y, min.z, 1.f };
		//glm::vec3 p4 = transform * glm::vec4{ min.x, max.y, max.z, 1.f };

		//glm::vec3 p5 = transform * glm::vec4{ max.x, min.y, min.z, 1.f };
		//glm::vec3 p6 = transform * glm::vec4{ max.x, min.y, max.z, 1.f };
		//glm::vec3 p7 = transform * glm::vec4{ max.x, max.y, min.z, 1.f };
		//glm::vec3 p8 = transform * glm::vec4{ max.x, max.y, max.z, 1.f };

		//s_debugRenderData->functionQueue.emplace_back([=](SceneRenderer::PerThreadData& data)
		//{
		//	// Bottom
		//	data.lineCommands.emplace_back(p1, p2, color);
		//	data.lineCommands.emplace_back(p2, p4, color);
		//	data.lineCommands.emplace_back(p4, p3, color);
		//	data.lineCommands.emplace_back(p3, p1, color);

		//	//// Top
		//	data.lineCommands.emplace_back(p5, p6, color);
		//	data.lineCommands.emplace_back(p6, p8, color);
		//	data.lineCommands.emplace_back(p8, p7, color);
		//	data.lineCommands.emplace_back(p7, p5, color);

		//	//// Connecting
		//	data.lineCommands.emplace_back(p1, p5, color);
		//	data.lineCommands.emplace_back(p2, p6, color);
		//	data.lineCommands.emplace_back(p3, p7, color);
		//	data.lineCommands.emplace_back(p4, p8, color);

		//});
	}

	void DebugRenderer::DrawSphere(const glm::vec3&, const glm::vec3&, const glm::vec4&)
	{
	}

	void DebugRenderer::DrawCube(const glm::vec3&, const glm::vec3&, const glm::vec3&, const glm::vec4&)
	{
	}
}
