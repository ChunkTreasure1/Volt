struct IndirectDispatchCommand
{
    uint x;
    uint y;
    uint z;
};

RWStructuredBuffer<IndirectDispatchCommand> u_commandsBuffer;

[numthreads(1, 1, 1)]
void MainWriteCommands(uint threadId : SV_DispatchThreadID)
{ 
    IndirectDispatchCommand result;
    result.x = 2;
    result.y = 1;
    result.z = 1;

    u_commandsBuffer[0] = result;
} 

#define GROUP_SIZE 32

RWStructuredBuffer<uint> u_outputBuffer;

[numthreads(GROUP_SIZE, 1, 1)]
void MainIndirectDispatch(uint threadId : SV_DispatchThreadID)
{
    u_outputBuffer[threadId] = threadId;
}