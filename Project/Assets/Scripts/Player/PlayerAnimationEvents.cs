using Volt;
using Volt.Audio;

namespace Project
{
    public class PlayerAnimationEvents : Script
    {
        AudioPlayerHandler myAudioHandler;

        void OnCreate()
        {
            myAudioHandler = entity.parent.parent.FindChild("Audio").GetScript<AudioPlayerHandler>();
        }

        void OnAnimationEvent(string eventName, uint frame)
        {
            //Log.Info(eventName);

            if (eventName == "Event_Footstep")
            {
                myAudioHandler.PlayFootstep();
            }
            if (eventName == "Event_Equip")
            {
                myAudioHandler.Equip();
            }
            if (eventName == "Event_Unequip")
            {
                myAudioHandler.Unequip();
            }
            if (eventName == "Event_Shotgun_Reload")
            {
                myAudioHandler.ReloadShotgun();
            }
            if (eventName == "Event_Shotgun_Pump")
            {
                myAudioHandler.Pump();
            }
            if (eventName == "Event_BoltAction_Bolt")
            {
                myAudioHandler.Bolt();
            }

        }
    }
}
