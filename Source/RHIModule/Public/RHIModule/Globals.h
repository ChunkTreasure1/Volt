#include <cstdint>

// Note: These are special values that can be used in shaders
//		 to define that the resource is a special type.
namespace Volt::RHI::Globals
{
	inline static constexpr uint32_t PUSH_CONSTANTS_BINDING = 999;
	inline static constexpr uint32_t RENDER_GRAPH_CONSTANTS_BINDING = 998;
	inline static constexpr uint32_t RENDER_GRAPH_CONSTANTS_SPACE = 1;
}
