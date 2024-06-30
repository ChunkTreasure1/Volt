#pragma once

float3 GetLightComplexityGradient(float value)
{
    float3 zero = float3(0.0, 0.0, 0.0);
    float3 white = float3(0.0, 0.1, 0.9);
    float3 red = float3(0.2, 0.9, 0.4);
    float3 blue = float3(0.8, 0.8, 0.3);
    float3 green = float3(0.9, 0.2, 0.3);

    float step0 = 0.0f;
    float step1 = 2.0f;
    float step2 = 8.0f;
    float step3 = 32.0f;
    float step4 = 64.0f;

    float3 color = lerp(zero, white, smoothstep(step0, step1, value));
    color = lerp(color, white, smoothstep(step1, step2, value));
    color = lerp(color, red, smoothstep(step1, step2, value));
    color = lerp(color, blue, smoothstep(step2, step3, value));
    color = lerp(color, green, smoothstep(step3, step4, value));

    return color;
}