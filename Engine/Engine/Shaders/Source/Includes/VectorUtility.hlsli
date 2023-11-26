#pragma once

float3x2 MakeF3X2FromColumns(float2 col0, float2 col1, float2 col2)
{
    return float3x2(col0, col1, col2);
}

float3x3 MakeF3X3FromRows(float3 row0, float3 row1, float3 row2)
{
    return transpose(float3x3(row0, row1, row2));
}

float3 GetRowInMatrix(in float3x2 m, uint row)
{
    return float3(m[0][row], m[1][row], m[2][row]);
}