using System;
using System.Collections.Generic;
using System.EnterpriseServices;
using System.Linq;
using System.Runtime.Caching;
using System.Web.Mvc.Html;
using Volt;

namespace Project
{
    public class Interactable_Door : Interactable_Cost
    {
        private Room myRoom;

        [RepNotify("OpenDoor")]
        public bool myOpen = false;

        private bool myOpenAnimationStarted = false;

        float slerpTimer = 0;
        float slerpDuration = 1;
        Quaternion startRotation;
        Quaternion endRotation;
        private void OnCreate()
        {
            if (entity.parent.HasScript<Room>())
            {
                myRoom = entity.parent.GetScript<Room>();
            }
        }

        public override void SpendEvent()
        {
            myOpen = true;
            Net.Notify(entity.Id, "myOpen");
            //OpenDoor();
        }


        public override void ShowUI(bool toggleVisibility)
        {
            if (toggleVisibility)
            {
                //Show appropriate UI elements
                if(cost == 500)
                {
                    UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.OpenDoorOne);
                }
                else if (cost == 750)
                {
                    UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.OpenDoorTwo);
                }
                else
                {
                    UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.OpenDoorThree);
                }


            }
            else if (!toggleVisibility)
            {
                //Get rid of UI element
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
            }
        }

        public void OpenDoor()
        {
            myRoom.IsUnlocked = true;
            myOpenAnimationStarted = true;
            UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);

            startRotation = entity.rotation;
            endRotation = startRotation * Quaternion.Euler(new Vector3(0, Mathf.Radians(170), 0));

            entity.RemoveComponent<BoxColliderComponent>();
            Entity.Destroy(entity.children[0]);
        }
        protected override void Update()
        {
            if(myOpenAnimationStarted)
            {
                if (slerpTimer < 1)
                {
                    entity.rotation = Quaternion.Slerp(startRotation, endRotation, slerpTimer);
                    slerpTimer += 2 * Time.deltaTime;
                }
                if (slerpTimer > 1)
                {
                    myOpen = false;
                    entity.rotation = endRotation;
                }
            }
        }
    }
}