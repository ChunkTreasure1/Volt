namespace Volt.Audio
{
    public static class AMP
    {
        public static bool PlayOneshotEvent(string aEventName)
        {
            return InternalCalls.AMP_PlayOneshotEvent(aEventName);
        }
    }

    public class AudioSourceComponent : Component
    {

        #region Event
        public uint PlayEvent(string aEventName)
        {
            return InternalCalls.AudioSourceComponent_PlayEvent(entity.Id, aEventName);
        }

        public bool PlayOneshotEvent(string aEventName)
        {
            return InternalCalls.AudioSourceComponent_PlayOneshotEvent(entity.Id, aEventName);
        }

        public bool StopEvent(uint aPlayingID)
        {
            return InternalCalls.AudioSourceComponent_StopEvent(entity.Id, aPlayingID);
        }

        public bool PauseEvent(uint aPlayingID)
        {
            return InternalCalls.AudioSourceComponent_PauseEvent(entity.Id, aPlayingID);
        }

        public bool ResumeEvent(uint aPlayingID)
        {
            return InternalCalls.AudioSourceComponent_PauseEvent(entity.Id, aPlayingID);
        }
        #endregion

        #region GameSyncs

        public bool SetState(string aStateGroup, string aState)
        {
            return InternalCalls.AudioSourceComponent_SetState(entity.Id, aStateGroup, aState);
        }

        public bool SetSwitch(string aStateGroup, string aState)
        {
            return InternalCalls.AudioSourceComponent_SetSwitch(entity.Id, aStateGroup, aState);
        }

        public bool SetParameter(string aParameterGroup, float aValue)
        {
            return InternalCalls.AudioSourceComponent_SetParameter(entity.Id, aParameterGroup, aValue);
        }

        public bool SetParameter(string aParameterGroup, float aValue, uint aOvertime)
        {
            return InternalCalls.AudioSourceComponent_SetParameterOverTime(entity.Id, aParameterGroup, aValue, aOvertime);
        }

        #endregion


    }
}
