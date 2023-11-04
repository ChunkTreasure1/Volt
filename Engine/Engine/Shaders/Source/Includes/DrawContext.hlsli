#pragma once

struct DrawContext
{
    TypedBuffer<uint> drawToInstanceOffset;
    TypedBuffer<uint> instanceOffsetToObjectIDBuffer;
};