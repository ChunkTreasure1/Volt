#include "vtpch.h"
#include "SubTexture2D.h"

#include "Volt/Rendering/Texture/Texture2D.h"

namespace Volt
{
	SubTexture2D::SubTexture2D(Ref<Texture2D> aTexture, const glm::vec2& aMin, const glm::vec2& aMax)
		: myTexture(aTexture)
	{
		myTexCoords[0] = { aMin.x, aMax.y };
		myTexCoords[1] = { aMax.x, aMax.y };
		myTexCoords[2] = { aMax.x, aMin.y };
		myTexCoords[3] = { aMin.x, aMin.y };
	}

	SubTexture2D SubTexture2D::CreateFromCoords(Ref<Texture2D> aTexture, const glm::vec2& aSpriteCoords, const glm::vec2& aCellSize, const glm::vec2& aSpriteSize)
	{
		const glm::vec2 min = { (aSpriteCoords.x * aCellSize.x) / aTexture->GetWidth(), (aSpriteCoords.y * aCellSize.y) / aTexture->GetHeight() };
		const glm::vec2 max = { ((aSpriteCoords.x + aSpriteSize.x) * aCellSize.x) / aTexture->GetWidth(), ((aSpriteCoords.y + aSpriteSize.y) * aCellSize.y) / aTexture->GetHeight() };

		return SubTexture2D(aTexture, min, max);
	}
}