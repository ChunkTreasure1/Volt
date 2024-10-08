#pragma once

bool IsBitSet(uint value, uint bit)
{
    return (value & bit) != 0;
}