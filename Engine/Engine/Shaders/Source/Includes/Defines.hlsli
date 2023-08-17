#ifndef DEFINES_H
#define DEFINES_H

#define BUILTIN_VARIABLE(builtInName, type, name) [[vk::builtin(builtInName)]] type name : name
#define PUSH_CONSTANT(type, name) [[vk::push_constant]] type name

#endif