#pragma once

#include "Volt/Core/Base.h"

#include <GEM/gem.h>

namespace Volt
{
	class Texture2D;
	class SubTexture2D
	{
	public:
		SubTexture2D() = default;
		SubTexture2D(Ref<Texture2D> aTexture, const gem::vec2& aMin, const gem::vec2& aMax);

		inline const Ref<Texture2D> GetTexture() const { return myTexture; }
		inline const gem::vec2* GetTexCoords() const { return myTexCoords; }

		static SubTexture2D CreateFromCoords(Ref<Texture2D> aTexture, const gem::vec2& aSpriteCoords, const gem::vec2& aCellSize, const gem::vec2& aSpriteSize = { 1, 1 });

	private:
		Ref<Texture2D> myTexture;
		gem::vec2 myTexCoords[4];
	};
}