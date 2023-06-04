using System.Collections.Generic;
using System.Linq;
using Volt;

namespace Project
{
    public abstract class Interactable_Base : Script
    {
        public bool isInRange = false;
        public float TimeToHold = 0.2f;

        private bool myHasBeenInteracted = false;
        private float myHoldTimer = 0f;
        private void OnCreate()
        {
            myHoldTimer = TimeToHold;
        }

        private void OnTriggerEnter(Entity other)
        {
            if(other.HasScript<Player>())
            {
                isInRange = true;
                Log.Info("Is In Range");
                ShowUI(isInRange);
            }
        }

        private void OnTriggerExit(Entity other)
        {
            if(other.HasScript<Player>())
            {
                myHasBeenInteracted = false;
                isInRange = false;
                Log.Info("Is Not Range");
                ShowUI(isInRange);
            }
        }
        private void OnUpdate(float deltaTime)
        {
            if (Input.IsKeyDown(KeyCode.F))
            {
                if (!myHasBeenInteracted && isInRange)
                {
                    if (myHoldTimer <= 0)
                    {
                        myHasBeenInteracted = true;
                        myHoldTimer = TimeToHold;
                        InteractEvent();
                    }
                    else
                    {
                        myHoldTimer -= Time.deltaTime;
                    }
                }
            }

            Update();
        }

        protected virtual void Update() { }

        public abstract void InteractEvent();
        public abstract void ShowUI(bool toggleVisibility);
    }
}
