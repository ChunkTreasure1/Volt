#ifndef DEFINES_H
#define DEFINES_H

#define STAGE_VARIABLE(type, name, semantic, loc) [[vk::location(loc)]] type name : semantic
#define BUILTIN_VARIABLE(builtInName, type, name) [[vk::builtin(builtInName)]] type name : name
#define CONSTANT(type, name, defaultValue, loc) [[vk::constant_id(loc)]] const type name = defaultValue

//// Set Defines /////
#define SPACE_TEXTURES space0
#define SPACE_MAINBUFFERS space1
#define SPACE_SAMPLERS space2
#define SPACE_RENDERER_BUFFERS space3

#define SPACE_OTHER space4

#define SPACE_PBR_RESOURCES space5
#define SPACE_MATERIAL space6
#define SPACE_DRAW_BUFFERS space7

///// Binding Defines /////

// Space 0
#define BINDING_TEXTURE2DTABLE t0
#define BINDING_TEXTURECUBETABLE t1
#define BINDING_TEXTURE3DTABLE t2

// Space 1
#define BINDING_MATERIALTABLE t0

// Space 2
#define SAMPLER_LINEAR s0
#define SAMPLER_LINEAR_POINT s1
#define SAMPLER_POINT s2
#define SAMPLER_POINT_LINEAR s3

#define SAMPLER_LINEAR_CLAMP s4
#define SAMPLER_LINEAR_POINT_CLAMP s5
#define SAMPLER_POINT_CLAMP s6
#define SAMPLER_POINT_LINEAR_CLAMP s7

#define SAMPLER_ANISO s8
#define SAMPLER_SHADOW s9
#define SAMPLER_REDUCE s10

// Space 3
#define BINDING_CAMERABUFFER b1
#define BINDING_INDIRECT_ARGS u2
#define BINDING_OBJECT_DATA t4
#define BINDING_DIRECTIONAL_LIGHT b6
#define BINDING_POINT_LIGHTS t7
#define BINDING_SCENE_DATA b8
#define BINDING_SPOT_LIGHTS t9
#define BINDING_ANIMATION_DATA t10
#define BINDING_SPHERE_LIGHTS t11
#define BINDING_RECTANGLE_LIGHTS t12
#define BINDING_PARTICLE_INFO t13
#define BINDING_VISIBLE_POINT_LIGHTS u14
#define BINDING_RENDERER_DATA b15
#define BINDING_PAINTED_VERTEX_COLORS t16
#define BINDING_VISIBLE_SPOT_LIGHTS u17

// Space 5
#define BINDING_BRDF t0
#define BINDING_IRRADIANCE t1
#define BINDING_RADIANCE t2
#define BINDING_DIR_SHADOWMAP t3
#define BINDING_SPOT_SHADOWMAPS t4
#define BINDING_POINT_SHADOWMAPS t5
#define BINDING_VOLUMETRIC_FOG_TEXTURE t6
#define BINDING_AO_TEXTURE t7

// Space 7
#define BINDING_INDIRECT_COUNT u0
#define BINDING_DRAW_TO_OBJECT_ID u1

#endif