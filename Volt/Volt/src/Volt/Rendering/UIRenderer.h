#pragma once

#include <RHIModule/Images/Image.h>

namespace Volt
{
	namespace RHI
	{
		class Image2D;
	}

	class Texture2D;
	class Font;

	class UIRenderer
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Begin(RefPtr<RHI::Image> renderTarget);
		static void End();

		static void SetView(const glm::mat4& viewMatrix);
		static void SetProjection(const glm::mat4& projectionMatrix);

		static void SetViewport(float x, float y, float width, float height);
		static void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

		static void DrawSprite(Ref<Texture2D> texture, const glm::vec3& position, const glm::vec2& scale, float rotation, const glm::vec4& color = 1.f, const glm::vec2& offset = 0.f);
		static void DrawSprite(RefPtr<RHI::Image> image, const glm::vec3& position, const glm::vec2& scale, float rotation, const glm::vec4& color = 1.f, const glm::vec2& offset = 0.f);
		static void DrawSprite(const glm::vec3& position, const glm::vec2& scale, float rotation, const glm::vec4& color, const glm::vec2& offset = 0.f);

		static void DrawString(const std::string& text, const Ref<Font> font, const glm::vec3& position, const glm::vec2& scale, float rotation, float maxWidth, const glm::vec4& color, const glm::vec2& positionOffset);
		static void DispatchText();
		static void DispatchSprites();

		static float GetCurrentScaleModifier();
		static float GetCurrentScaleModifierX();

	private:
		static void CreateQuadData();
		static void CreateTextData();

		UIRenderer() = delete;
	};
}
