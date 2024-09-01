#include "sbpch.h"
#include "CoreUtilities/util_dll.h"
#include "Volt/util_static_lib1.h"

int main(int, char**)
{
    std::cout << "Hello Events World, from" << std::endl;

#if defined(_DEBUG) && _DEBUG
    std::cout << "- Exe is built in Debug"
#  if defined(USES_FASTBUILD) && USES_FASTBUILD
        " with FastBuild"
#  endif
        "!" << std::endl;
#endif

#if defined(NDEBUG) && NDEBUG
    std::cout << "- Exe is built in Release"
#  if defined(USES_FASTBUILD) && USES_FASTBUILD
        " with FastBuild"
#  endif
        "!" << std::endl;
#endif

    std::vector<int> someArray(5, 6);

    // from dll1
    UtilDll1 utilityDll;
    utilityDll.ComputeSum(someArray);

    system("PAUSE");

    return 0;
}
