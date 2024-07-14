#pragma once

#if __VULKAN__

#define PUSH_CONSTANT(type, name) [[vk::push_constant]] type name
#define BUILTIN_VARIABLE(builtInName, type, name) [[vk::builtin(builtInName)]] type name : name

#elif __D3D12__
#define PUSH_CONSTANT(type, name) ConstantBuffer<type> name : register(b999, space0)
#endif