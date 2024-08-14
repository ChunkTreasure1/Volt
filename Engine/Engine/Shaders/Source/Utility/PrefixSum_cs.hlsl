#include "Defines.hlsli"
#include "Resources.hlsli"

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

struct Constants
{
    vt::UniformTypedBuffer<uint> inputValues;
    vt::UniformRWTypedBuffer<uint> outputValues;
    vt::UniformRWTypedBuffer<State> stateBuffer; // Should be globallycoherent
    uint valueCount;
};

groupshared uint m_wavePrefixSums[TG_WAVE_COUNT];
groupshared uint m_groupAggregate;

[numthreads(TG_SIZE, 1, 1)]
void main(uint3 groupThreadId : SV_GroupThreadID, uint3 groupId : SV_GroupID)
{
    const Constants constants = GetConstants<Constants>();
    
    const uint WAVE_SIZE = WaveGetLaneCount();
    const uint WAVE_INDEX = groupThreadId.x / WAVE_SIZE;
    const uint LANE_INDEX = WaveGetLaneIndex();

    const uint localValueIndex = WAVE_SIZE * WAVE_INDEX + LANE_INDEX;
    const uint valueIndex = TG_SIZE * groupId.x + localValueIndex;

    if (valueIndex >= constants.valueCount)
    {
        return;
    }
    
    const bool isLastLaneInWave = groupThreadId.x == (WAVE_INDEX * WAVE_SIZE) + WAVE_SIZE - 1;
    
    uint value = constants.inputValues.Load(valueIndex);
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
        State state;
        state.aggregate = laneAggregate + value;
        state.state = STATE_AGG_READY;
        
        [branch]
        if (groupId.x == 0)
        {
            state.prefix = laneAggregate;
            state.state = STATE_PRE_READY;
        }
        
        constants.stateBuffer.Store(groupId.x, state);
    }
    
    if (groupThreadId.x < groupId.x && groupId.x != 0)
    {
        int currentLookBackIndex = groupId.x - groupThreadId.x;
        while (currentLookBackIndex >= 0)
        {
            // #TODO_Ivar: Implement better lookback
            uint localLookBackState = constants.stateBuffer.Load(currentLookBackIndex).state;
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
                uint otherAggregate = constants.stateBuffer.Load(currentLookBackIndex).aggregate;
                InterlockedAdd(m_groupAggregate, otherAggregate);
                
                currentLookBackIndex -= TG_SIZE;
                continue;
            }
        }
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    if (groupThreadId.x == 0)
    {
        State state = constants.stateBuffer.Load(groupId.x);
        state.state = STATE_PRE_READY;

        constants.stateBuffer.Store(groupId.x, state);
    }
    
    constants.outputValues.Store(valueIndex, m_groupAggregate + laneAggregate);
}