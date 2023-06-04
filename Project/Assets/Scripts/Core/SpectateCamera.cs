using System.Collections.Generic;
using Volt;

namespace Project
{
    public class SpectateCamera : Script
    {
        public bool Active 
        {
            get
            {
                return myIsActive;
            }

            set
            {
                if(myPlayers == null || myPlayers.Count == 0) { myIsActive = false; return; }

                if(value == true)
                {
                    myIsActive = true;
                    Vision.SetActiveCamera(entity.Id);
                    Vision.SetCameraFollow(entity, myPlayers[0]);
                    return;
                }

                myIsActive = false;
            }
        }

        private bool myIsActive = false;

        private int myActivePlayer = 0;
        private List<Entity> myPlayers;

        private void OnCreate()
        {
            myPlayers = new List<Entity>();

            foreach(Entity player in Scene.GetAllEntitiesWithScript<Player>())
            {
                if (player.HasScript<PlayerInputHandler>()) { continue; }

                myPlayers.Add(player);
            }
        }

        private void OnUpdate(float deltaTime)
        {
            if (Input.IsKeyPressed(KeyCode.U))
            {
                //DEBUG! DEBUG! DEBUG! DEBUG! DEBUG! DEBUG! DEBUG! DEBUG!
                Active = true;
            }

            if (Active)
            {
                if (Input.IsKeyPressed(KeyCode.Space))
                {
                    myActivePlayer++;
                    if(myActivePlayer >= myPlayers.Count)
                    {
                        myActivePlayer = 0;
                    }

                    Vision.SetCameraFollow(entity, myPlayers[myActivePlayer]);
                }
            }
        }
    }
}
