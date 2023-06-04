using Volt;

namespace Project
{
    public class EndGameCamera : Script
    {
        private Entity myStartCam;
        private Entity myEndCam;

        private bool myEndingStarted = false;

        private void OnCreate()
        {
            myStartCam = entity.FindChild("StartCam");
            myEndCam = entity.FindChild("EndCam");

            GameManager.Instance.EndGameEvent += StartEndSequence;
        }

        public void StartEndSequence()
        {
            if(myStartCam != null)
            {
                Vision.SetActiveCamera(myStartCam.Id);
                myEndingStarted = true;

                entity.CreateTimer(0.01f, () => { Vision.SetActiveCamera(myEndCam.Id); });
            }
        }
    }
}
