#include "Defines.hlsli"

#ifndef SAMPLERSTATES_H
#define SAMPLERSTATES_H

SamplerState u_linearSampler : register(SAMPLER_LINEAR, SPACE_SAMPLERS);
SamplerState u_linearPointSampler : register(SAMPLER_LINEAR_POINT, SPACE_SAMPLERS);

SamplerState u_pointSampler : register(SAMPLER_POINT, SPACE_SAMPLERS);
SamplerState u_pointLinearSampler : register(SAMPLER_POINT_LINEAR, SPACE_SAMPLERS);

SamplerState u_linearSamplerClamp : register(SAMPLER_LINEAR_CLAMP, SPACE_SAMPLERS);
SamplerState u_linearPointSamplerClamp : register(SAMPLER_LINEAR_POINT_CLAMP, SPACE_SAMPLERS);

SamplerState u_pointSamplerClamp : register(SAMPLER_POINT_CLAMP, SPACE_SAMPLERS);
SamplerState u_pointLinearSamplerClamp : register(SAMPLER_POINT_LINEAR_CLAMP, SPACE_SAMPLERS);

SamplerState u_anisotropicSampler : register(SAMPLER_ANISO, SPACE_SAMPLERS);
SamplerComparisonState u_shadowSampler : register(SAMPLER_SHADOW, SPACE_SAMPLERS);

SamplerState u_reduceSampler : register(SAMPLER_REDUCE, SPACE_SAMPLERS);

#endif