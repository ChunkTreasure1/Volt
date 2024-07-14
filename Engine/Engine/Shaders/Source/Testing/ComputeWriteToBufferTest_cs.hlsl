RWStructuredBuffer<uint> u_outputBuffer;

#define GROUP_SIZE 32

[numthreads(GROUP_SIZE, 1, 1)]
void main(uint threadId : SV_DispatchThreadID)
{ 
    u_outputBuffer[threadId] = threadId;
} 