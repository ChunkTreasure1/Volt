#pragma once

struct DrawContext
{
    TypedBuffer<uint> drawToInstanceOffset;
    TypedBuffer<uint> instanceOffsetToObjectIDBuffer;
    
    TypedBuffer<uint> drawIndexToObjectId;
    TypedBuffer<uint> drawIndexToMeshletId;
};