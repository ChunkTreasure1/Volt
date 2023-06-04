using System.Collections.Generic;
using System.Linq;
using Volt;

namespace Project
{
    public class UnderPass_Door : Script
    {
        protected bool isInRange = false;

        private void OnTriggerEnter(Entity other)
        {
            if (other.HasScript<Player>())
            {
                isInRange = true;
                ShowUI(isInRange);
            }
        }

        private void OnTriggerExit(Entity other)
        {
            if (other.HasScript<Player>())
            {
                isInRange = false;
                ShowUI(isInRange);
            }
        }
        private void ShowUI(bool toggleVisibility)
        {
            if (toggleVisibility)
            {
                //Show appropriate UI elements
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.NoPower);

            }
            else if (!toggleVisibility)
            {
                //Get rid of UI element
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
            }
        }

        public void OpenDoor()
        {
            UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
            Entity.Destroy(entity);
        }
    }
}
