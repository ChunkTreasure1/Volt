#pragma once

#include "Utility.hlsli"

#define WRAPPED_GROUP_STRIDE 128

#define MAX_DISPATCH_THREAD_GROUPS_X 65535
#define MAX_DISPATCH_THREAD_GROUPS_Y 65535
#define MAX_DISPATCH_THREAD_GROUPS_Z 65535

uint3 GetGroupCountWrapped(uint targetGroupCount)
{
    uint3 groupCount = uint3(targetGroupCount, 1, 1);
    
    if (groupCount.x > MAX_DISPATCH_THREAD_GROUPS_X)
    {
        groupCount.y = DivideRoundUp(groupCount.x, WRAPPED_GROUP_STRIDE);
        groupCount.x = WRAPPED_GROUP_STRIDE;
    }

    if (groupCount.y > MAX_DISPATCH_THREAD_GROUPS_Y)
    {
        groupCount.z = DivideRoundUp(groupCount.y, WRAPPED_GROUP_STRIDE);
        groupCount.y = WRAPPED_GROUP_STRIDE;
    }

    return groupCount;
}

uint3 GetGroupCountWrapped(uint threadCount, uint groupSize)
{
    return GetGroupCountWrapped(DivideRoundUp(threadCount, groupSize));
}

uint UnwrapDispatchGroupId(uint3 groupId)
{
    return groupId.x + (groupId.z * WRAPPED_GROUP_STRIDE + groupId.y) * WRAPPED_GROUP_STRIDE;
}

uint UnwrapDispatchThreadId(uint3 groupId, uint groupThreadIndex, uint threadGroupSize)
{
    return UnwrapDispatchGroupId(groupId) * threadGroupSize + groupThreadIndex;
}