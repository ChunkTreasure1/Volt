using Volt;
using Volt.Audio;

namespace Project
{
    public enum InstanceTrigger
    {
        None,
        Awake,
        TriggerEnter,
        TriggerExit,
        ColliderEnter,
        ColliderExit
    };


    public class AudioEmitter : Script
    {
        public InstanceTrigger myStartTrigger = InstanceTrigger.None;
        public InstanceTrigger myStopTrigger = InstanceTrigger.None;

        public WWiseEvents myEvent = WWiseEvents.Play_Music_3D;

        public bool myTriggerOnce = false;

        private bool isTriggered = false;
        private AudioSourceComponent myAudioSource;

        private uint myPlayingID = 0;

        private void OnCreate()
        {
            myAudioSource = entity.GetComponent<AudioSourceComponent>();
        }
        
        private void OnAwake()
        {
            if(myStartTrigger == InstanceTrigger.Awake)
            {
                StartEvent();
            }
        }

        private void OnTriggerEnter(Entity other)
        {
            if (isTriggered && myTriggerOnce) { return; }

            if (myStartTrigger == InstanceTrigger.TriggerEnter)
            {
                StartEvent();
            }

            if (myStopTrigger == InstanceTrigger.TriggerEnter)
            {
                StopEvent();
            }
        }

        private void OnTriggerExit(Entity other)
        {
            if (isTriggered && myTriggerOnce) { return; }
            if (myStartTrigger == InstanceTrigger.TriggerExit)
            {
                StartEvent();
            }

            if (myStopTrigger == InstanceTrigger.TriggerExit)
            {
                StopEvent();
            }
        }

        private void OnCollisionEnter(Entity other)
        {
            if (isTriggered && myTriggerOnce) { return; }
            if (myStartTrigger == InstanceTrigger.ColliderEnter)
            {
                StartEvent();
            }

            if (myStopTrigger == InstanceTrigger.ColliderEnter)
            {
                StopEvent();
            }
        }

        private void OnCollisionExit(Entity other)
        {
            if (isTriggered && myTriggerOnce) { return; }
            if (myStartTrigger == InstanceTrigger.ColliderExit)
            {
                StartEvent();
            }

            if (myStopTrigger == InstanceTrigger.ColliderExit)
            {
                StopEvent();
            }
        }

        private void StartEvent()
        {
            myPlayingID = entity.GetComponent<AudioSourceComponent>().PlayEvent(myEvent.ToString());
            isTriggered = true;
        }

        private void StopEvent()
        {
            entity.GetComponent<AudioSourceComponent>().StopEvent(myPlayingID);
            isTriggered = true;
        }
    }
}
