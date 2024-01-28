#include "cupch.h"
#include "CoreUtilities/GUIDUtilities.h"

#ifdef VT_PLATFORM_WINDOWS

#include <rpc.h>

namespace GUIDUtilities
{
	VoltGUID GenerateGUID()
	{
		GUID newGuid;
		HRESULT result = CoCreateGuid(&newGuid);
		result;
		assert(SUCCEEDED(result));

		return VoltGUID::Construct(newGuid.Data1, newGuid.Data2, newGuid.Data3,
			newGuid.Data4[0], newGuid.Data4[1], newGuid.Data4[2], newGuid.Data4[3], newGuid.Data4[4],
			newGuid.Data4[5], newGuid.Data4[6], newGuid.Data4[7]);
	}
}

#endif
