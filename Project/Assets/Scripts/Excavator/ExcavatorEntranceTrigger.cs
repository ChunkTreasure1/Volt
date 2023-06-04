using Volt;

namespace Project
{
    public class ExcavatorEntranceTrigger : Script
    {
        public Font TextFont;

        private bool myPlayerInTrigger = false;
        private Entity myPlayer;

        private void OnUpdate(float deltaTime)
        {
            if (myPlayerInTrigger)
            {
                if (Input.IsKeyPressed(KeyCode.F))
                {
                    entity.parent.GetScript<ExcavatorController>().Enter(myPlayer);
                }
            }
        }

        private void OnTriggerEnter(Entity other)
        {
            if (other.HasScript<Player>())
            {
                myPlayer = other;
                myPlayerInTrigger = true;
            }
        }

        private void OnTriggerExit(Entity other) 
        { 
            if (other.HasScript<Player>()) 
            {
                myPlayer = null;
                myPlayerInTrigger = false; 
            } 
        }

        private void OnRenderUI()
        {
            if (myPlayerInTrigger && TextFont != null)
            {
                UIRenderer.DrawString("Press F to enter", TextFont, new Vector3(-100f, 0f, 10f), new Vector2(50f), 0f, 1000f, new Vector4(1f));
            }
        }
    }
}
