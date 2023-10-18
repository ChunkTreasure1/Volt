#include "Defines.hlsli"

#define TG_SIZE 512
#define TG_WAVE_SIZE 32
#define TG_WAVE_COUNT (TG_SIZE / TG_WAVE_SIZE)

#define STATE_NOT_READY 0
#define STATE_AGG_READY 1
#define STATE_PRE_READY 2

struct State
{
    uint aggregate;
    uint prefix;
    uint state;
};

struct Data
{
    uint valueCount;
};

PUSH_CONSTANT(Data, u_data);

StructuredBuffer<uint> u_inputValues : register(t0, space0);

RWStructuredBuffer<uint> o_outputBuffer : register(u1, space0);
globallycoherent RWStructuredBuffer<State> o_stateBuffer : register(u2, space0);

groupshared uint m_wavePrefixSums[TG_WAVE_COUNT];
groupshared uint m_groupAggregate;

[numthreads(TG_SIZE, 1, 1)]
void main(uint3 groupThreadId : SV_GroupThreadID, uint3 groupId : SV_GroupID)
{
    const uint WAVE_SIZE = WaveGetLaneCount();
    const uint WAVE_INDEX = groupThreadId.x / WAVE_SIZE;
    const uint LANE_INDEX = WaveGetLaneIndex();

    const uint localValueIndex = WAVE_SIZE * WAVE_INDEX + LANE_INDEX;
    const uint valueIndex = TG_SIZE * groupId.x + localValueIndex;

    if (valueIndex >= u_data.valueCount)
    {
        return;
    }
    
    const bool isLastLaneInWave = groupThreadId.x == (WAVE_INDEX * WAVE_SIZE) + WAVE_SIZE - 1;
    
    uint value = u_inputValues[valueIndex];
    uint lanePrefixSum = WavePrefixSum(value);

    if (isLastLaneInWave)
    {
        m_wavePrefixSums[WAVE_INDEX] = lanePrefixSum + value;
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    if (groupThreadId.x == 0)
    {
        [unroll]
        for (uint i = 1; i < TG_WAVE_COUNT; i++)
        {
            m_wavePrefixSums[i] += m_wavePrefixSums[i - 1];
        }
        
        m_groupAggregate = 0;
    }
    
    GroupMemoryBarrierWithGroupSync();

    uint laneAggregate = lanePrefixSum;
    
    if (WAVE_INDEX > 0)
    {
        laneAggregate = laneAggregate + m_wavePrefixSums[WAVE_INDEX - 1];
    }
    
    if (groupThreadId.x == TG_SIZE - 1)
    {
        o_stateBuffer[groupId.x].aggregate = laneAggregate + value;
        o_stateBuffer[groupId.x].state = STATE_AGG_READY;
        
        [branch]
        if (groupId.x == 0)
        {
            o_stateBuffer[groupId.x].prefix = laneAggregate;
            o_stateBuffer[groupId.x].state = STATE_PRE_READY;
        }
    }
    
    if (groupThreadId.x < groupId.x && groupId.x != 0)
    {
        int currentLookBackIndex = groupId.x - groupThreadId.x;
        while (currentLookBackIndex >= 0)
        {
            // #TODO_Ivar: Implement better lookback
            uint localLookBackState = o_stateBuffer[currentLookBackIndex].state;
            //if (localLookBackState == STATE_PRE_READY)
            //{
            //    //uint otherPrefix  = o_stateBuffer[currentLookBackIndex].prefix;
            //    //InterlockedAdd(m_groupAggregate, otherPrefix);
                
            //    //currentLookBackIndex -= TG_SIZE;
            //    //continue;

            //}
            //else 
            if (localLookBackState == STATE_AGG_READY || localLookBackState == STATE_PRE_READY)
            {
                uint otherAggregate = o_stateBuffer[currentLookBackIndex].aggregate;
                InterlockedAdd(m_groupAggregate, otherAggregate);
                
                currentLookBackIndex -= TG_SIZE;
                continue;
            }
        }
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    if (groupThreadId.x == 0)
    {
        o_stateBuffer[groupId.x].state = STATE_PRE_READY;
    }
    
    o_outputBuffer[valueIndex] = m_groupAggregate + laneAggregate;
}