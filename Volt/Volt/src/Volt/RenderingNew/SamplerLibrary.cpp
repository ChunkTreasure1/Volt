#include "vtpch.h"
#include "SamplerLibrary.h"

#include <VoltRHI/Images/SamplerState.h>

namespace Volt
{
	void SamplerLibrary::Add(SamplerType samplerType)
	{
		RHI::SamplerStateCreateInfo info{};
	
		switch (samplerType)
		{
			case SamplerType::Linear:
				info.minFilter = RHI::TextureFilter::Linear;
				info.magFilter = RHI::TextureFilter::Linear;
				info.mipFilter = RHI::TextureFilter::Linear;
				info.wrapMode = RHI::TextureWrap::Repeat;
				break;

			case SamplerType::LinearClamp:
				info.minFilter = RHI::TextureFilter::Linear;
				info.magFilter = RHI::TextureFilter::Linear;
				info.mipFilter = RHI::TextureFilter::Linear;
				info.wrapMode = RHI::TextureWrap::Clamp;
				break;

			case SamplerType::LinearPoint:
				info.minFilter = RHI::TextureFilter::Linear;
				info.magFilter = RHI::TextureFilter::Nearest;
				info.mipFilter = RHI::TextureFilter::Nearest;
				info.wrapMode = RHI::TextureWrap::Repeat;
				break;

			case SamplerType::LinearPointClamp:
				info.minFilter = RHI::TextureFilter::Linear;
				info.magFilter = RHI::TextureFilter::Nearest;
				info.mipFilter = RHI::TextureFilter::Nearest;
				info.wrapMode = RHI::TextureWrap::Clamp;
				break;

			case SamplerType::Point:
				info.minFilter = RHI::TextureFilter::Nearest;
				info.magFilter = RHI::TextureFilter::Nearest;
				info.mipFilter = RHI::TextureFilter::Nearest;
				info.wrapMode = RHI::TextureWrap::Repeat;
				break;

			case SamplerType::PointClamp:
				info.minFilter = RHI::TextureFilter::Nearest;
				info.magFilter = RHI::TextureFilter::Nearest;
				info.mipFilter = RHI::TextureFilter::Nearest;
				info.wrapMode = RHI::TextureWrap::Clamp;
				break;

			case SamplerType::PointLinear:
				info.minFilter = RHI::TextureFilter::Nearest;
				info.magFilter = RHI::TextureFilter::Linear;
				info.mipFilter = RHI::TextureFilter::Linear;
				info.wrapMode = RHI::TextureWrap::Repeat;
				break;

			case SamplerType::PointLinearClamp:
				info.minFilter = RHI::TextureFilter::Nearest;
				info.magFilter = RHI::TextureFilter::Linear;
				info.mipFilter = RHI::TextureFilter::Linear;
				info.wrapMode = RHI::TextureWrap::Clamp;
				break;

			case SamplerType::Aniso2:
				info.minFilter = RHI::TextureFilter::Linear;
				info.magFilter = RHI::TextureFilter::Linear;
				info.mipFilter = RHI::TextureFilter::Linear;
				info.wrapMode = RHI::TextureWrap::Repeat;
				info.anisotropyLevel = RHI::AnisotropyLevel::X2;
				break;

			case SamplerType::Aniso4:
				info.minFilter = RHI::TextureFilter::Linear;
				info.magFilter = RHI::TextureFilter::Linear;
				info.mipFilter = RHI::TextureFilter::Linear;
				info.wrapMode = RHI::TextureWrap::Repeat;
				info.anisotropyLevel = RHI::AnisotropyLevel::X4;
				break;

			case SamplerType::Aniso8:
				info.minFilter = RHI::TextureFilter::Linear;
				info.magFilter = RHI::TextureFilter::Linear;
				info.mipFilter = RHI::TextureFilter::Linear;
				info.wrapMode = RHI::TextureWrap::Repeat;
				info.anisotropyLevel = RHI::AnisotropyLevel::X8;
				break;

			case SamplerType::Aniso16:
				info.minFilter = RHI::TextureFilter::Linear;
				info.magFilter = RHI::TextureFilter::Linear;
				info.mipFilter = RHI::TextureFilter::Linear;
				info.wrapMode = RHI::TextureWrap::Repeat;
				info.anisotropyLevel = RHI::AnisotropyLevel::X16;
				break;

			case SamplerType::Shadow:
				info.minFilter = RHI::TextureFilter::Linear;
				info.magFilter = RHI::TextureFilter::Linear;
				info.mipFilter = RHI::TextureFilter::Linear;
				info.wrapMode = RHI::TextureWrap::Repeat;
				info.compareOperator = RHI::CompareOperator::LessEqual;
				break;
		}

		m_samplerStates[samplerType] = RHI::SamplerState::Create(info);
	}

	Ref<RHI::SamplerState> SamplerLibrary::Get(SamplerType samplerType)
	{
		if (m_samplerStates.contains(samplerType))
		{
			return m_samplerStates.at(samplerType);
		}

		return nullptr;
	}
}
