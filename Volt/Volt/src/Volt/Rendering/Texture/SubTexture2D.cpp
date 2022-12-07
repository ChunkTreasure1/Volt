#include "vtpch.h"
#include "SubTexture2D.h"

#include "Volt/Rendering/Texture/Texture2D.h"

namespace Volt
{
	SubTexture2D::SubTexture2D(Ref<Texture2D> aTexture, const gem::vec2& aMin, const gem::vec2& aMax)
		: myTexture(aTexture)
	{
		myTexCoords[0] = { aMin.x, aMax.y };
		myTexCoords[1] = { aMax.x, aMax.y };
		myTexCoords[2] = { aMax.x, aMin.y };
		myTexCoords[3] = { aMin.x, aMin.y };
	}

	SubTexture2D SubTexture2D::CreateFromCoords(Ref<Texture2D> aTexture, const gem::vec2& aSpriteCoords, const gem::vec2& aCellSize, const gem::vec2& aSpriteSize)
	{
		const gem::vec2 min = { (aSpriteCoords.x * aCellSize.x) / aTexture->GetWidth(), (aSpriteCoords.y * aCellSize.y) / aTexture->GetHeight() };
		const gem::vec2 max = { ((aSpriteCoords.x + aSpriteSize.x) * aCellSize.x) / aTexture->GetWidth(), ((aSpriteCoords.y + aSpriteSize.y) * aCellSize.y) / aTexture->GetHeight() };

		return SubTexture2D(aTexture, min, max);
	}
}