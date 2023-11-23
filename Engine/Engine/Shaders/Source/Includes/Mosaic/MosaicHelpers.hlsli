#pragma once

template<typename T, typename L>
T Add(in T lhs, in L rhs)
{
    return lhs + (T)(rhs);
}

template<typename T, typename L>
T Subtract(in T lhs, in L rhs)
{
    return lhs - (T)(rhs);
}

template<typename T, typename L>
T Multiply(in T lhs, in L rhs)
{
    return lhs * (T)rhs;
}

template<typename T, typename L>
T Divide(in T numerator, in L denominator)
{
    return numerator / denominator;
}

