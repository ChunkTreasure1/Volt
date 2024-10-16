#pragma once

#include "RenderCore/Config.h"

#include <SubSystem/SubSystem.h>

#include <CoreUtilities/Pointers/RefPtr.h>

namespace Volt
{
	namespace RHI
	{
		class ShaderCompiler;
		class ShaderCache;
	}

	class VTRC_API ShaderSubSystem : public SubSystem
	{
	public:
		void Initialize() override;
		void Shutdown() override;

		VT_DECLARE_SUBSYSTEM("{B017EA3B-6D55-46B8-BEDE-A299C3580B4E}"_guid);
	private:
		RefPtr<RHI::ShaderCompiler> m_shaderCompiler;
		RefPtr<RHI::ShaderCache> m_shaderCache;
	};
}
