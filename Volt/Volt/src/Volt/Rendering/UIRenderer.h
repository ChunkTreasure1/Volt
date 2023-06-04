#pragma once

namespace Volt
{
	class Texture2D;
	class Image2D;
	class Font;

	class UIRenderer
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Begin(Ref<Image2D> renderTarget);
		static void End();

		static void SetView(const gem::mat4& viewMatrix);
		static void SetProjection(const gem::mat4& projectionMatrix);

		static void SetViewport(float x, float y, float width, float height);
		static void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

		static void DrawSprite(Ref<Texture2D> texture, const gem::vec3& position, const gem::vec2& scale, float rotation, const gem::vec4& color = 1.f, const gem::vec2& offset = 0.f);
		static void DrawSprite(const gem::vec3& position, const gem::vec2& scale, float rotation, const gem::vec4& color, const gem::vec2& offset = 0.f);

		static void DrawString(const std::string& text, const Ref<Font> font, const gem::vec3& position, const gem::vec2& scale, float rotation, float maxWidth, const gem::vec4& color);
		static void DispatchText();

		static float GetCurrentScaleModifier();

	private:
		static void CreateQuadData();
		static void CreateTextData();

		UIRenderer() = delete;
	};
}
