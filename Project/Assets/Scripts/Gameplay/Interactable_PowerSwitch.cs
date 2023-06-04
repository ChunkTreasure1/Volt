using System.Collections.Generic;
using System.EnterpriseServices;
using System.Linq;
using System.Web.Mvc.Html;
using Volt;
using Volt.Audio;

namespace Project
{
    public class Interactable_PowerSwitch : Interactable_Base
    {
        public override void InteractEvent()
        {
            ActivatePower();
        }

        public override void ShowUI(bool toggleVisibility)
        {
            if (toggleVisibility)
            {
                //Show appropriate UI elements
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Power);
            }
            else if (!toggleVisibility)
            {
                //Get rid of UI element
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
            }
        }

        private void ActivatePower()
        {
            //Call net event that phone active!

            Entity[] phoneBooth = Scene.GetAllEntitiesWithScript<Interactable_PhoneBooth>();
            Entity[] underpassDoors = Scene.GetAllEntitiesWithScript<UnderPass_Door>();
            Entity[] underPassLights = Scene.GetAllEntitiesWithScript<UnderpassLight>();

            if (phoneBooth != null && phoneBooth.Length > 0)
            {
                phoneBooth[0].GetScript<Interactable_PhoneBooth>().Activate();
            }
            foreach(Entity entity in underpassDoors) 
            {
                entity.GetScript<UnderPass_Door>().OpenDoor();
            }
            foreach(Entity entity in underPassLights)
            {
                entity.GetScript<UnderpassLight>().TurnOnLights();
            }

            UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);

            if (entity.HasComponent<AnimationControllerComponent>())
            {
                entity.GetComponent<AnimationControllerComponent>().controller?.SetParameter("PowerOn", true);
            }

            entity.GetComponent<AudioSourceComponent>().PlayEvent(WWiseEvents.Play_Switch.ToString());

        }
    }
}
