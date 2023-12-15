#pragma once

struct DrawContext
{
    TypedBuffer<uint> instanceOffsetToObjectIDBuffer;
    
    TypedBuffer<uint> drawIndexToObjectId;
    TypedBuffer<uint> drawIndexToMeshletId;
};