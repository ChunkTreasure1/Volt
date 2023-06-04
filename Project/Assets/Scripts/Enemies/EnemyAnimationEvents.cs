using Volt;
using Volt.Audio;

namespace Project
{
    public class EnemyAnimationEvents : Script
    {
        AudioEnemyHandler myAudioHandler;

        void OnCreate()
        {
            myAudioHandler = entity.parent.FindChild("Audio").GetScript<AudioEnemyHandler>();
        }

        void OnAnimationEvent(string eventName, uint frame)
        {
            if (eventName == "Event_Footstep")
            {
                myAudioHandler.PlayFootstep();
            }

            if (eventName == "Event_StopAttack")
            {
                NetEvents.EventFromLocalId(entity.Id, eNetEvent.Animation, (byte)eEnemyAnimation.Attack, false);
            }
        }
    }
}