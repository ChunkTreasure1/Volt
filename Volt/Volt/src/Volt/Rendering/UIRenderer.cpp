#include "vtpch.h"
#include "UIRenderer.h"

#include "Volt/Core/Application.h"
#include "Volt/Core/Window.h"

#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/SubMaterial.h"
#include "Volt/Asset/Text/Font.h"
#include "Volt/Asset/Text/MSDFData.h"

#include "Volt/Rendering/Renderer.h"

#include "Volt/Rendering/Buffer/VertexBufferSet.h"

#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/RenderPipeline/ShaderRegistry.h"

#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/Texture/TextureTable.h"

#include "Volt/Rendering/Vertex.h"

#include "Volt/Utility/StringUtility.h"

#include <VoltRHI/Buffers/IndexBuffer.h>
#include <VoltRHI/Buffers/VertexBuffer.h>

namespace Volt
{
	struct SpriteCommand
	{
		Ref<Image2D> image;
		glm::vec4 color;
		glm::vec3 position;
		glm::vec2 scale;
		glm::vec2 offset;
		float rotation;
	};

	struct QuadData
	{
		Ref<RHI::VertexBuffer> vertexBuffer;
		Ref<RHI::IndexBuffer> indexBuffer;
		Ref<Material> quadMaterial;
	};

	struct TextData
	{
		inline ~TextData()
		{
			if (!vertexBufferBase)
			{
				return;
			}

			delete[] vertexBufferBase;
			vertexBufferBase = nullptr;
			vertexBufferPtr = nullptr;
		}

		inline static constexpr uint32_t MAX_QUADS = 10000;
		inline static constexpr uint32_t MAX_VERTICES = MAX_QUADS * 4;
		inline static constexpr uint32_t MAX_INDICES = MAX_QUADS * 6;

		Ref<VertexBufferSet> vertexBuffer;
		Ref<RHI::IndexBuffer> indexBuffer;
		Ref<RenderPipeline> renderPipeline;

		glm::vec4 vertices[4];
		uint32_t indexCount = 0;

		TextVertex* vertexBufferBase = nullptr;
		TextVertex* vertexBufferPtr = nullptr;
	};

	struct UIRendererData
	{
		QuadData quadData;
		TextData textData;

		Weak<Image2D> currentRenderTarget;
		glm::mat4 currentProjection = { 1.f };
		glm::mat4 currentView = { 1.f };

		Ref<CommandBuffer> commandBuffer;
		Ref<Image2D> depthImage;

		std::vector<SpriteCommand> spriteCommands;
	};

	Scope<UIRendererData> s_uiRendererData;
	inline static const glm::vec2 CORE_SIZE = { 1920.f / 2.f, 1080.f / 2.f };

	inline static float Remap(float value, float fromMin, float fromMax, float toMin, float toMax)
	{
		float fromAbs = value - fromMin;
		float fromMaxAbs = fromMax - fromMin;

		float normal = fromAbs / fromMaxAbs;

		float toMaxAbs = toMax - toMin;
		float toAbs = toMaxAbs * normal;

		float to = toAbs + toMin;

		return to;
	}

	void UIRenderer::Initialize()
	{
		//const uint32_t framesInFlight = Application::Get().GetWindow().GetSwapchain().GetMaxFramesInFlight();
		s_uiRendererData = CreateScope<UIRendererData>();
		s_uiRendererData->currentView = { 1.f };

		CreateQuadData();
		CreateTextData();

		// Depth image
		{
			ImageSpecification spec{};
			spec.width = 1280;
			spec.height = 720;
			spec.format = ImageFormat::DEPTH32F;
			spec.usage = ImageUsage::Attachment;
			spec.debugName = "UI Depth";

			s_uiRendererData->depthImage = Image2D::Create(spec);
		}

		//s_uiRendererData->commandBuffer = CommandBuffer::Create(framesInFlight);
	}

	void UIRenderer::Shutdown()
	{
		s_uiRendererData = nullptr;
	}

	void UIRenderer::Begin(Ref<Image2D> renderTarget)
	{
		VT_PROFILE_FUNCTION();

		if (s_uiRendererData->depthImage->GetWidth() != renderTarget->GetWidth() || s_uiRendererData->depthImage->GetHeight() != renderTarget->GetHeight())
		{
			s_uiRendererData->depthImage->Invalidate(renderTarget->GetWidth(), renderTarget->GetHeight());
		}

		s_uiRendererData->commandBuffer->Begin();
		s_uiRendererData->currentRenderTarget = renderTarget;

		renderTarget->TransitionToLayout(s_uiRendererData->commandBuffer->GetCurrentCommandBuffer(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		s_uiRendererData->depthImage->TransitionToLayout(s_uiRendererData->commandBuffer->GetCurrentCommandBuffer(), VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

		Renderer::BeginSection(s_uiRendererData->commandBuffer, "UI", { 0.5f, 0.5f, 0.5f, 1.f });

		VkRenderingAttachmentInfo colorAttInfo{};
		colorAttInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttInfo.imageView = renderTarget->GetView();
		colorAttInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttInfo.clearValue.color = { 0.f, 0.f, 0.f, 1.f };
		colorAttInfo.clearValue.depthStencil = { 1.f, 0 };

		VkRenderingAttachmentInfo depthAttInfo{};
		depthAttInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttInfo.imageView = s_uiRendererData->depthImage->GetView();
		depthAttInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depthAttInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttInfo.clearValue.color = { 0.f, 0.f, 0.f, 1.f };
		depthAttInfo.clearValue.depthStencil = { 1.f, 0 };

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = { 0, 0, renderTarget->GetWidth(), renderTarget->GetHeight() };
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttInfo;
		renderingInfo.pDepthAttachment = &depthAttInfo;
		renderingInfo.pStencilAttachment = nullptr;

		SetViewport(0.f, 0.f, (float)renderTarget->GetWidth(), (float)renderTarget->GetHeight());
		SetScissor(0, 0, renderTarget->GetWidth(), renderTarget->GetHeight());

		vkCmdBeginRendering(s_uiRendererData->commandBuffer->GetCurrentCommandBuffer(), &renderingInfo);

		//s_uiRendererData->quadData.vertexBuffer->Bind(s_uiRendererData->commandBuffer->GetCurrentCommandBuffer());
		//s_uiRendererData->quadData.indexBuffer->Bind(s_uiRendererData->commandBuffer->GetCurrentCommandBuffer());
	}

	void UIRenderer::End()
	{
		VT_PROFILE_FUNCTION();

		vkCmdEndRendering(s_uiRendererData->commandBuffer->GetCurrentCommandBuffer());
		Renderer::EndSection(s_uiRendererData->commandBuffer);

		s_uiRendererData->currentRenderTarget->TransitionToLayout(s_uiRendererData->commandBuffer->GetCurrentCommandBuffer(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		s_uiRendererData->commandBuffer->End();
		s_uiRendererData->commandBuffer->Submit();

		// Reset text
		{
			s_uiRendererData->textData.vertexBufferPtr = s_uiRendererData->textData.vertexBufferBase;
			s_uiRendererData->textData.indexCount = 0;
		}
	}

	void UIRenderer::SetView(const glm::mat4& viewMatrix)
	{
		s_uiRendererData->currentView = viewMatrix;
	}

	void UIRenderer::SetProjection(const glm::mat4& projectionMatrix)
	{
		s_uiRendererData->currentProjection = projectionMatrix;
	}

	void UIRenderer::SetViewport(float x, float y, float width, float height)
	{
		VkViewport viewport{};
		viewport.x = x;
		viewport.y = y;
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		vkCmdSetViewport(s_uiRendererData->commandBuffer->GetCurrentCommandBuffer(), 0, 1, &viewport);
	}

	void UIRenderer::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
	{
		VkRect2D rect{};
		rect.offset.x = x;
		rect.offset.y = y;
		rect.extent.width = width;
		rect.extent.height = height;

		vkCmdSetScissor(s_uiRendererData->commandBuffer->GetCurrentCommandBuffer(), 0, 1, &rect);
	}

	void UIRenderer::DrawSprite(Ref<Texture2D> texture, const glm::vec3& position, const glm::vec2& scale, float rotation, const glm::vec4& color, const glm::vec2& offset)
	{
		VT_PROFILE_FUNCTION();

		auto& newCmd = s_uiRendererData->spriteCommands.emplace_back();
		//newCmd.image = texture ? texture->GetImage() : nullptr;
		newCmd.position = position;
		newCmd.scale = scale;
		newCmd.rotation = rotation;
		newCmd.color = color;
		newCmd.offset = offset;
	}

	void UIRenderer::DrawSprite(Ref<Image2D> image, const glm::vec3& position, const glm::vec2& scale, float rotation, const glm::vec4& color, const glm::vec2& offset)
	{
		VT_PROFILE_FUNCTION();

		auto& newCmd = s_uiRendererData->spriteCommands.emplace_back();
		newCmd.image = image;
		newCmd.position = position;
		newCmd.scale = scale;
		newCmd.rotation = rotation;
		newCmd.color = color;
		newCmd.offset = offset;
	}

	void UIRenderer::DrawSprite(const glm::vec3& position, const glm::vec2& scale, float rotation, const glm::vec4& color, const glm::vec2& offset)
	{
		DrawSprite(Ref<Image2D>(nullptr), position, scale, rotation, color, offset);
	}

	static bool NextLine(int32_t aIndex, const std::vector<int32_t>& aLines)
	{
		for (int32_t line : aLines)
		{
			if (line == aIndex)
			{
				return true;
			}
		}

		return false;
	}

	void UIRenderer::DrawString(const std::string& text, const Ref<Font> font, const glm::vec3& position, const glm::vec2& scale, float rotation, float maxWidth, const glm::vec4& color, const glm::vec2& positionOffset)
	{
		VT_PROFILE_FUNCTION();

		auto& textData = s_uiRendererData->textData;

		std::u32string utf32string = ::Utility::To_UTF32(text);
		Ref<Texture2D> fontAtlas = font->GetAtlas();

		if (!fontAtlas)
		{
			return;
		}

		uint32_t textureIndex = 0; // Renderer::GetBindlessData().textureTable->GetBindingFromTexture(fontAtlas->GetImage());
		if (textureIndex == 0)
		{
			//textureIndex = Renderer::GetBindlessData().textureTable->AddTexture(fontAtlas->GetImage());
		}

		auto& fontGeom = font->GetMSDFData()->fontGeometry;
		const auto& metrics = fontGeom.getMetrics();

		std::vector<int32_t> nextLines;

		// Find new lines
		{
			double x = 0.0;
			double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
			double y = -fsScale * metrics.ascenderY;

			int32_t lastSpace = -1;

			for (int32_t i = 0; i < utf32string.size(); i++)
			{
				char32_t character = utf32string[i];
				if (character == '\n')
				{
					x = 0.0;
					y -= fsScale * metrics.lineHeight;
					continue;
				}

				auto glyph = fontGeom.getGlyph(character);
				if (!glyph)
				{
					glyph = fontGeom.getGlyph('?');
				}

				if (!glyph)
				{
					continue;
				}

				if (character != ' ')
				{
					// Calculate geometry
					double pl, pb, pr, pt;
					glyph->getQuadPlaneBounds(pl, pb, pr, pt);
					glm::vec2 quadMin((float)pl, (float)pb);
					glm::vec2 quadMax((float)pl, (float)pb);

					quadMin *= (float)fsScale;
					quadMax *= (float)fsScale;
					quadMin += glm::vec2((float)x, (float)y);
					quadMax += glm::vec2((float)x, (float)y);

					if (quadMax.x > maxWidth && lastSpace != -1)
					{
						i = lastSpace;
						nextLines.emplace_back(lastSpace);
						lastSpace = -1;
						x = 0.0;
						y -= fsScale * metrics.lineHeight;
					}
				}
				else
				{
					lastSpace = i;
				}

				double advance = glyph->getAdvance();
				fontGeom.getAdvance(advance, character, utf32string[i + 1]);
				x += fsScale * advance;
			}
		}

		// Setup vertices
		{
			double x = 0.0;
			double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
			double y = 0.0;

			const glm::vec2 currentSize = { (float)s_uiRendererData->currentRenderTarget->GetWidth(), (float)s_uiRendererData->currentRenderTarget->GetHeight() };
			const glm::vec2 halfSize = currentSize / 2.f;

			const glm::vec3 remappedPosition = { Remap(position.x, -CORE_SIZE.x, CORE_SIZE.x, -halfSize.x, halfSize.x), Remap(position.y, -CORE_SIZE.y, CORE_SIZE.y, -halfSize.y, halfSize.y), position.z };
			const glm::vec2 newScale = { currentSize.x / CORE_SIZE.x, currentSize.y / CORE_SIZE.y };
			const float minScale = glm::min(newScale.x, newScale.y);

			glm::mat4 transform = glm::translate(glm::mat4{ 1.f }, glm::vec3{ remappedPosition.x, remappedPosition.y, remappedPosition.z })* glm::rotate(glm::mat4{ 1.f }, rotation, { 0.f, 0.f, 1.f })* glm::scale(glm::mat4{ 1.f }, { scale.x * minScale, scale.y * minScale, 1.f });
			transform[3][0] -= positionOffset.x;
			transform[3][1] -= positionOffset.y;

			for (int32_t i = 0; i < utf32string.size(); i++)
			{
				char32_t character = utf32string[i];
				if (character == '\n' || NextLine(i, nextLines))
				{
					x = 0.0;
					y -= fsScale * metrics.lineHeight;
					continue;
				}

				auto glyph = fontGeom.getGlyph(character);

				if (!glyph)
				{
					glyph = fontGeom.getGlyph('?');
				}

				if (!glyph)
				{
					continue;
				}

				double l, b, r, t;
				glyph->getQuadAtlasBounds(l, b, r, t);

				double pl, pb, pr, pt;
				glyph->getQuadPlaneBounds(pl, pb, pr, pt);

				pl *= fsScale, pb *= fsScale, pr *= fsScale, pt *= fsScale;
				pl += x, pb += y, pr += x, pt += y;

				double texelWidth = 1.0 / fontAtlas->GetWidth();
				double texelHeight = 1.0 / fontAtlas->GetHeight();

				l *= texelWidth, b *= texelHeight, r *= texelWidth, t *= texelHeight;

				textData.vertexBufferPtr->position = transform * glm::vec4{ (float)pl, (float)pb, 0.f, 1.f };
				textData.vertexBufferPtr->color = color;
				textData.vertexBufferPtr->texCoords = { (float)l, (float)b };
				textData.vertexBufferPtr->textureIndex = (uint32_t)textureIndex;
				textData.vertexBufferPtr++;

				textData.vertexBufferPtr->position = transform * glm::vec4{ (float)pr, (float)pb, 0.f, 1.f };
				textData.vertexBufferPtr->color = color;
				textData.vertexBufferPtr->texCoords = { (float)r, (float)b };
				textData.vertexBufferPtr->textureIndex = (uint32_t)textureIndex;
				textData.vertexBufferPtr++;

				textData.vertexBufferPtr->position = transform * glm::vec4{ (float)pr, (float)pt, 0.f, 1.f };
				textData.vertexBufferPtr->color = color;
				textData.vertexBufferPtr->texCoords = { (float)r, (float)t };
				textData.vertexBufferPtr->textureIndex = (uint32_t)textureIndex;
				textData.vertexBufferPtr++;

				textData.vertexBufferPtr->position = transform * glm::vec4{ (float)pl, (float)pt, 0.f, 1.f };
				textData.vertexBufferPtr->color = color;
				textData.vertexBufferPtr->texCoords = { (float)l, (float)t };
				textData.vertexBufferPtr->textureIndex = (uint32_t)textureIndex;
				textData.vertexBufferPtr++;

				textData.indexCount += 6;

				double advance = glyph->getAdvance();
				fontGeom.getAdvance(advance, character, utf32string[i + 1]);
				x += fsScale * advance;
			}
		}
	}

	void UIRenderer::DispatchText()
	{
		VT_PROFILE_FUNCTION();

		auto& textData = s_uiRendererData->textData;

		if (textData.indexCount == 0)
		{
			return;
		}

		const uint32_t currentIndex = s_uiRendererData->commandBuffer->GetCurrentIndex();
		auto vertexBuffer = textData.vertexBuffer->Get(currentIndex);

		//uint32_t dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(textData.vertexBufferPtr) - reinterpret_cast<uint8_t*>(textData.vertexBufferBase));
		//vertexBuffer->SetDataMapped(s_uiRendererData->commandBuffer->GetCurrentCommandBuffer(), textData.vertexBufferBase, dataSize);

		//const glm::mat4 viewProjection = s_uiRendererData->currentProjection * s_uiRendererData->currentView;
		//Renderer::DrawIndexedVertexBuffer(s_uiRendererData->commandBuffer, textData.indexCount, vertexBuffer, textData.indexBuffer, textData.renderPipeline, &viewProjection, sizeof(glm::mat4));
	}

	void UIRenderer::DispatchSprites()
	{
		std::sort(s_uiRendererData->spriteCommands.begin(), s_uiRendererData->spriteCommands.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs.position.z > rhs.position.z;
		});

		for (const auto& cmd : s_uiRendererData->spriteCommands)
		{
			auto subMat = s_uiRendererData->quadData.quadMaterial->GetSubMaterialAt(0);

			const glm::vec2 currentSize = { (float)s_uiRendererData->currentRenderTarget->GetWidth(), (float)s_uiRendererData->currentRenderTarget->GetHeight() };
			const glm::vec2 halfSize = currentSize / 2.f;

			const glm::vec3 remappedPosition = { Remap(cmd.position.x, -CORE_SIZE.x, CORE_SIZE.x, -halfSize.x, halfSize.x), Remap(cmd.position.y, -CORE_SIZE.y, CORE_SIZE.y, -halfSize.y, halfSize.y), cmd.position.z };
			const glm::vec2 newScale = { currentSize.x / CORE_SIZE.x, currentSize.y / CORE_SIZE.y };
			const float minScale = glm::min(newScale.x, newScale.y);

			const glm::mat4 transform = glm::translate(glm::mat4{ 1.f }, remappedPosition)* glm::rotate(glm::mat4{ 1.f }, cmd.rotation, { 0.f, 0.f, 1.f })* glm::scale(glm::mat4{ 1.f }, { cmd.scale.x * minScale, cmd.scale.y * minScale, 1.f });

			subMat->Bind(s_uiRendererData->commandBuffer);

			if (cmd.image)
			{
				uint32_t textureIndex = Renderer::GetBindlessData().textureTable->GetBindingFromTexture(cmd.image);
				if (textureIndex == 0)
				{
					textureIndex = Renderer::GetBindlessData().textureTable->AddTexture(cmd.image);
				}

				subMat->SetValue("textureIndex", textureIndex);
			}
			else
			{
				subMat->SetValue("textureIndex", 0);
			}

			subMat->SetValue("color", cmd.color);
			subMat->SetValue("viewProjectionTransform", s_uiRendererData->currentProjection * s_uiRendererData->currentView * transform);
			subMat->SetValue("vertexOffset", cmd.offset);
			subMat->PushMaterialData(s_uiRendererData->commandBuffer);

			//const uint32_t currentIndex = Application::Get().GetWindow().GetSwapchain().GetCurrentFrame();

			//if (subMat->GetPipeline()->HasDescriptorSet(Sets::SAMPLERS))
			//{
			//	auto descriptorSet = Renderer::GetBindlessData().globalDescriptorSets[Sets::SAMPLERS]->GetOrAllocateDescriptorSet(currentIndex);
			//	subMat->GetPipeline()->BindDescriptorSet(s_uiRendererData->commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::SAMPLERS);
			//}

			//if (subMat->GetPipeline()->HasDescriptorSet(Sets::TEXTURES))
			//{
			//	auto descriptorSet = Renderer::GetBindlessData().globalDescriptorSets[Sets::TEXTURES]->GetOrAllocateDescriptorSet(currentIndex);
			//	subMat->GetPipeline()->BindDescriptorSet(s_uiRendererData->commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::TEXTURES);
			//}

			vkCmdDrawIndexed(s_uiRendererData->commandBuffer->GetCurrentCommandBuffer(), 6, 1, 0, 0, 0);
		}

		s_uiRendererData->spriteCommands.clear();
	}

	float UIRenderer::GetCurrentScaleModifier()
	{
		if (!s_uiRendererData->currentRenderTarget)
		{
			return 0.f;
		}

		const glm::vec2 currentSize = { (float)s_uiRendererData->currentRenderTarget->GetWidth(), (float)s_uiRendererData->currentRenderTarget->GetHeight() };
		const glm::vec2 newScale = { currentSize.x / CORE_SIZE.x, currentSize.y / CORE_SIZE.y };
		const float minScale = glm::min(newScale.x, newScale.y);

		return minScale;
	}

	float UIRenderer::GetCurrentScaleModifierX()
	{
		if (!s_uiRendererData->currentRenderTarget)
		{
			return 0.f;
		}

		const glm::vec2 currentSize = { (float)s_uiRendererData->currentRenderTarget->GetWidth(), (float)s_uiRendererData->currentRenderTarget->GetHeight() };
		const glm::vec2 newScale = { currentSize.x / CORE_SIZE.x, currentSize.y / CORE_SIZE.y };

		return newScale.x;
	}

	void UIRenderer::CreateQuadData()
	{
		RenderPipelineSpecification spec{};
		spec.name = "Quad";
		spec.shader = ShaderRegistry::GetShader("Quad");
		spec.depthMode = DepthMode::ReadWrite;
		spec.cullMode = CullMode::None;

		spec.framebufferAttachments =
		{
			{ ImageFormat::RGBA16F, { 1.f }, TextureBlend::Alpha },
			{ ImageFormat::DEPTH32F }
		};
		spec.vertexLayout =
		{
			{ ElementType::Float4, "POSITION" },
			{ ElementType::Float4, "COLOR" },
			{ ElementType::Float2, "TEXCOORD" }
		};

		spec.depthCompareOperator = CompareOperator::LessEqual;

		s_uiRendererData->quadData.quadMaterial = Material::Create(spec);

		// Vertex Buffer
		{
			constexpr uint32_t VERTEX_COUNT = 4;

			SpriteVertex* tempVertPtr = new SpriteVertex[VERTEX_COUNT];
			tempVertPtr[0].position = { -0.5f, -0.5f, 0.f, 1.f };
			tempVertPtr[1].position = { 0.5f, -0.5f, 0.f, 1.f };
			tempVertPtr[2].position = { 0.5f,  0.5f, 0.f, 1.f };
			tempVertPtr[3].position = { -0.5f,  0.5f, 0.f, 1.f };

			tempVertPtr[0].texCoords = { 0.f, 1.f };
			tempVertPtr[1].texCoords = { 1.f, 1.f };
			tempVertPtr[2].texCoords = { 1.f, 0.f };
			tempVertPtr[3].texCoords = { 0.f, 0.f };

			tempVertPtr[0].color = { 1.f };
			tempVertPtr[1].color = { 1.f };
			tempVertPtr[2].color = { 1.f };
			tempVertPtr[3].color = { 1.f };

			s_uiRendererData->quadData.vertexBuffer = RHI::VertexBuffer::Create(tempVertPtr, sizeof(SpriteVertex) * VERTEX_COUNT);
			delete[] tempVertPtr;
		}

		// Index Buffer
		{
			constexpr uint32_t INDEX_COUNT = 6;

			uint32_t* tempIndexPtr = new uint32_t[INDEX_COUNT];

			tempIndexPtr[0] = 0;
			tempIndexPtr[1] = 3;
			tempIndexPtr[2] = 2;

			tempIndexPtr[3] = 2;
			tempIndexPtr[4] = 1;
			tempIndexPtr[5] = 0;

			s_uiRendererData->quadData.indexBuffer = RHI::IndexBuffer::Create(tempIndexPtr, INDEX_COUNT);
			delete[] tempIndexPtr;
		}
	}

	void UIRenderer::CreateTextData()
	{
		auto& textData = s_uiRendererData->textData;

		// Create pipeline
		{
			RenderPipelineSpecification spec{};
			spec.name = "Text";
			spec.shader = ShaderRegistry::GetShader("Text");
			spec.depthMode = DepthMode::Read;
			spec.cullMode = CullMode::Back;

			spec.framebufferAttachments =
			{
				{ ImageFormat::RGBA16F, { 1.f }, TextureBlend::Alpha },
				{ ImageFormat::DEPTH32F }
			};

			spec.vertexLayout = TextVertex::GetVertexLayout();
			spec.depthCompareOperator = CompareOperator::LessEqual;

			textData.renderPipeline = RenderPipeline::Create(spec);
		}

		textData.vertexBufferBase = new TextVertex[TextData::MAX_INDICES];
		textData.vertexBufferPtr = textData.vertexBufferBase;

		textData.vertexBuffer = VertexBufferSet::Create(Renderer::GetFramesInFlightCount(), textData.vertexBufferBase, sizeof(TextVertex) * TextData::MAX_VERTICES);

		// Index buffer
		{
			uint32_t* indices = new uint32_t[TextData::MAX_INDICES];
			uint32_t offset = 0;

			for (uint32_t indice = 0; indice < TextData::MAX_INDICES; indice += 6)
			{
				indices[indice + 0] = offset + 0;
				indices[indice + 1] = offset + 3;
				indices[indice + 2] = offset + 2;

				indices[indice + 3] = offset + 2;
				indices[indice + 4] = offset + 1;
				indices[indice + 5] = offset + 0;

				offset += 4;
			}

			textData.indexBuffer = RHI::IndexBuffer::Create(indices, TextData::MAX_INDICES);
			delete[] indices;
		}
	}
}
