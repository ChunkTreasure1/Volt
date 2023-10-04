struct PushConstants
{
    uint size;
};

[[vk::push_constant]] PushConstants u_pushConstants;

RWStructuredBuffer<uint> u_countsBuffer : register(u0, space0);

[numthreads(256, 1, 1)]
void main(uint threadId : SV_DispatchThreadID)
{
    if (threadId >= u_pushConstants.size)
    {
        return;
    }
    
    u_countsBuffer[0] = 0;
    u_countsBuffer[1] = 0;
}