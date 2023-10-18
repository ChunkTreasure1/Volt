#pragma once

#define PUSH_CONSTANT(type, name) [[vk::push_constant]] type name
#define BUILTIN_VARIABLE(builtInName, type, name) [[vk::builtin(builtInName)]] type name : name