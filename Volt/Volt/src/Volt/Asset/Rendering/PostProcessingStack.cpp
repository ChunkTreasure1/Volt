#include "vtpch.h"
#include "PostProcessingStack.h"

namespace Volt
{
	void PostProcessingStack::PushEffect(PostProcessingEffect material)
	{
		myPostProcessingStack.emplace_back(material);
	}

	void PostProcessingStack::PopEffect()
	{
		if (!myPostProcessingStack.empty())
		{
			myPostProcessingStack.pop_back();
		}
	}
	 
	void PostProcessingStack::InsertEffect(PostProcessingEffect material, uint32_t index)
	{
		if (index >= myPostProcessingStack.size())
		{
			index = myPostProcessingStack.empty() ? 0 : static_cast<uint32_t>(myPostProcessingStack.size());
		}

		if (index == myPostProcessingStack.size())
		{
			myPostProcessingStack.emplace_back(material);
			return;
		}

		myPostProcessingStack.insert(myPostProcessingStack.begin() + index, material);
	}

	void PostProcessingStack::RemoveEffect(uint32_t index)
	{
		if (index >= myPostProcessingStack.size())
		{
			return;
		}

		myPostProcessingStack.erase(myPostProcessingStack.begin() + index);
	}
}
