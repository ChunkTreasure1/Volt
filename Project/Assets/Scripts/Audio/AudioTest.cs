using Volt;
using Volt.Audio;
namespace Project
{
    public class AudioTest : Script
    {
        public WWiseEvents myEvent;
        public uint playingID;

        private AudioSourceComponent myAudioSource;

        private void OnCreate()
        {
           myAudioSource = entity.GetComponent<AudioSourceComponent>();
            {
                playingID = myAudioSource.PlayEvent(myEvent.ToString());
            }
        }
    }
}
