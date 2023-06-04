using Volt;
using Volt.Audio;

namespace Project
{
    public class AudioPhoneBooth : Script
    {
        EntityTimer ringTimer;

        public void ActivateRinging()
        {
            if (entity.HasComponent<AudioSourceComponent>())
            {
                entity.GetComponent<AudioSourceComponent>().PlayEvent(WWiseEvents.Play_Phone_Far.ToString());
                ringTimer = entity.CreateTimer(15.0f, Ring);
            }
        }

        public void CancelRinging()
        {
            entity.CancelTimer(ringTimer);
        }

        void Ring()
        {
            entity.GetComponent<AudioSourceComponent>().PlayEvent(WWiseEvents.Play_Phone.ToString());
            ringTimer = entity.CreateTimer(15.0f, Ring);
        }
    }
}
