#include "Resources.hlsli"
#include "Structures.hlsli"

struct Constants
{
    vt::RWTypedBuffer<uint> countCommandBuffer;
    vt::RWTypedBuffer<MeshTaskCommand> taskCommands;
};

[numthreads(32, 1, 1)]
void MainCS(uint groupThreadId : SV_GroupThreadID)
{
    const Constants constants = GetConstants<Constants>();

    uint commandCount = constants.countCommandBuffer.Load(0);
    
    if (groupThreadId == 0)
    {
        //constants.countCommandBuffer.Store(1, min(commandCount, 65535));
        //constants.countCommandBuffer.Store(2, 1);
        //constants.countCommandBuffer.Store(3, 1);
        constants.countCommandBuffer.Store(1, min((commandCount + 31) / 32, 65535));
        constants.countCommandBuffer.Store(2, 32);
        constants.countCommandBuffer.Store(3, 1);
    }

    uint boundary = (commandCount + 31) & ~31;
    
    MeshTaskCommand dummyCommand;
    
    if (commandCount + groupThreadId < boundary)
    {
        constants.taskCommands.Store(commandCount + groupThreadId, dummyCommand);
    }
}