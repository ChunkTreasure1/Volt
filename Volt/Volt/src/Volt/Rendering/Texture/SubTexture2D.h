#pragma once

#include "Volt/Core/Base.h"

#include <glm/glm.hpp>

namespace Volt
{
	class Texture2D;
	class SubTexture2D
	{
	public:
		SubTexture2D() = default;
		SubTexture2D(Ref<Texture2D> aTexture, const glm::vec2& aMin, const glm::vec2& aMax);

		inline const Ref<Texture2D> GetTexture() const { return myTexture; }
		inline const glm::vec2* GetTexCoords() const { return myTexCoords; }

		static SubTexture2D CreateFromCoords(Ref<Texture2D> aTexture, const glm::vec2& aSpriteCoords, const glm::vec2& aCellSize, const glm::vec2& aSpriteSize = { 1, 1 });

	private:
		Ref<Texture2D> myTexture;
		glm::vec2 myTexCoords[4];
	};
}