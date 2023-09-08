#include "Defines.glsl"

#ifndef SAMPLERSTATES_H
#define SAMPLERSTATES_H

layout(set = SPACE_SAMPLERS, binding = SAMPLER_LINEAR) uniform sampler u_linearSampler;
layout(set = SPACE_SAMPLERS, binding = SAMPLER_LINEAR_POINT) uniform sampler u_linearPointSampler;

layout(set = SPACE_SAMPLERS, binding = SAMPLER_POINT) uniform sampler u_pointSampler;
layout(set = SPACE_SAMPLERS, binding = SAMPLER_POINT_LINEAR) uniform sampler u_pointLinearSampler;

layout(set = SPACE_SAMPLERS, binding = SAMPLER_LINEAR_CLAMP) uniform sampler u_linearSamplerClamp;
layout(set = SPACE_SAMPLERS, binding = SAMPLER_LINEAR_POINT_CLAMP) uniform sampler u_linearPointSamplerClamp;

layout(set = SPACE_SAMPLERS, binding = SAMPLER_POINT_CLAMP) uniform sampler u_pointSamplerClamp;
layout(set = SPACE_SAMPLERS, binding = SAMPLER_POINT_LINEAR_CLAMP) uniform sampler u_pointLinearSamplerClamp;

layout(set = SPACE_SAMPLERS, binding = SAMPLER_ANISO) uniform sampler u_anisotropicSampler;
layout(set = SPACE_SAMPLERS, binding = SAMPLER_SHADOW) uniform sampler u_shadowSampler;

#endif