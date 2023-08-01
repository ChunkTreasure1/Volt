#pragma once
#include "Nexus/API/API.h"
#include "Nexus/Core/Result.h"

namespace Nexus
{
	class Session
	{
	public:
		static void Start();
		static void Clean();

		static bool IsValid();
		static int LastError();

		static void ErrorPrint();
		static Result GetResult();
		static const NXS_API_UTYPE_SESSION& GetRaw();

	private:
		inline static NXS_API_UTYPE_SESSION m_session;
		inline static Result m_result;
		inline static bool m_isValid = false;
	};
}
