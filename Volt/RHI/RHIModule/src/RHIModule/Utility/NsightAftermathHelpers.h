#pragma once

#include "RHIModule/Core/Core.h"

#ifdef VT_ENABLE_NV_AFTERMATH

#include "GFSDK_Aftermath.h"
#include "GFSDK_Aftermath_GpuCrashDump.h"

#include <iomanip>
#include <string>


namespace std
{
	template<typename T>
	inline std::string to_hex_string(T n)
	{
		std::stringstream stream;
		stream << std::setfill('0') << std::setw(2 * sizeof(T)) << std::hex << n;
		return stream.str();
	}

	inline std::string to_string(GFSDK_Aftermath_Result result)
	{
		return std::string("0x") + to_hex_string(static_cast<uint32_t>(result));
	}
} // namespace std

namespace Volt::RHI
{
	class AftermathException : public std::runtime_error
	{
	public:
		AftermathException(GFSDK_Aftermath_Result result)
			: std::runtime_error(GetErrorMessage(result)), m_result(result)
		{
		}

		AftermathException Error() const
		{
			return m_result;
		}

		static std::string GetErrorMessage(GFSDK_Aftermath_Result result)
		{
			switch (result)
			{
				case GFSDK_Aftermath_Result_FAIL_DriverVersionNotSupported:
					return "Unsupported driver version - requires an NVIDIA R495 display driver or newer.";
				case GFSDK_Aftermath_Result_FAIL_D3dDllInterceptionNotSupported:
					return "Aftermath is incompatible with D3D API interception, such as PIX or Nsight Graphics.";
				case GFSDK_Aftermath_Result_NotAvailable:
					return "Aftermath functionality is not availiable.";
				case GFSDK_Aftermath_Result_Fail:
					return "Unspecified Aftermath failure.";
				case GFSDK_Aftermath_Result_FAIL_VersionMismatch:
					return "Aftermath version mismatch.";
				case GFSDK_Aftermath_Result_FAIL_NotInitialized:
					return "Aftermath has not been initialized.";
				case GFSDK_Aftermath_Result_FAIL_InvalidAdapter:
					return "Device is not supported by Aftermath.";
				case GFSDK_Aftermath_Result_FAIL_InvalidParameter:
					return "Invalid parameter was supplied.";
				case GFSDK_Aftermath_Result_FAIL_Unknown:
					return "Unknown Aftermath failure.";
				case GFSDK_Aftermath_Result_FAIL_ApiError:
					return "Aftermath API failure.";
				case GFSDK_Aftermath_Result_FAIL_NvApiIncompatible:
					return "NV API not compatible.";
				case GFSDK_Aftermath_Result_FAIL_GettingContextDataWithNewCommandList:
					return "Trying to fetch aftermath data in an invalid context.";
				case GFSDK_Aftermath_Result_FAIL_AlreadyInitialized:
					return "Aftermath has already been initialized.";


				default:
					return "Aftermath Error 0x" + std::to_hex_string(result);
			}
		}

	private:
		const GFSDK_Aftermath_Result m_result;
	};

#define AFTERMATH_CHECK_ERROR(FC)																			\
	[&]() {                                                                                                 \
		GFSDK_Aftermath_Result _result = FC;                                                                \
		if (!GFSDK_Aftermath_SUCCEED(_result))                                                              \
		{                                                                                                   \
			VT_LOGC(LogVerbosity::Error, LogRender, "Error: {0}", AftermathException::GetErrorMessage(_result).c_str()); \
			throw AftermathException(_result);                                                              \
		}                                                                                                   \
	}()
}

#endif
