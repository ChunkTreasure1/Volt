using System;
using System.Collections.Generic;
using Volt;

namespace Project
{
    public class BackgroundCameras : Script
    {
        List<Entity[]> myCameraSequences;
        Entity myCurrentLastInSeq;

        int myCurrentSequence = 0;

        private void OnCreate()
        {
            myCameraSequences = new List<Entity[]>();

            foreach(Entity cameraGroup in entity.children)
            {
                if (cameraGroup.children.Length > 0)
                {
                    Entity[] cameras;
                    cameras = cameraGroup.children;
                    myCameraSequences.Add(cameras);
                }
            }

            myCurrentLastInSeq = myCameraSequences[0][myCameraSequences[0].Length - 1];
            myCurrentSequence = 0;

            Vision.SetActiveCamera(myCameraSequences[myCurrentSequence][1].Id);
        }

        private void OnUpdate(float deltaTime)
        {
            if(Vision.GetActiveCamera() == myCurrentLastInSeq)
            {
                myCurrentSequence++;

                if(myCurrentSequence >= myCameraSequences.Count)
                {
                    myCurrentSequence = 0;
                }

                myCurrentLastInSeq = myCameraSequences[myCurrentSequence][myCameraSequences[myCurrentSequence].Length - 1];

                Vision.SetActiveCamera(myCameraSequences[myCurrentSequence][0].Id);

                return;
            }
        }
    }
}
