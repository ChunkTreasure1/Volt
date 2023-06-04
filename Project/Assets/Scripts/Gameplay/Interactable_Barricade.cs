using System.Collections.Generic;
using System.Linq;
using Volt;
using Volt.Audio;

namespace Project
{
    public class Interactable_Barricade : Interactable_Base
    {
        [RepContinuous]
        public int Health = 5;

        private Entity[] myPlanks;

        AudioSourceComponent myAudioSource;

        private float myTimeBtwRepairs = 1.3f;

        public override void InteractEvent()
        {
            RepairBarricade();
        }

        private void RepairBarricade()
        {
            Health += 1;
            PointManager.Instance?.AddPoints(10);

            if(Health >= 5)
            {
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
            }
        }
        public void TakeDamage()
        {
            Health -= 1;
            myAudioSource.PlayEvent(WWiseEvents.Play_Barricade_Break.ToString());
        }

        public bool IsBroken()
        {
            return Health <= 0;
        }

        public void OnCarpenterEvent()
        {
            Health = 5;
        }

        private void OnCreate()
        {
            myPlanks = new Entity[5];
            myPlanks[0] = entity.FindChild("Plank_01");
            myPlanks[1] = entity.FindChild("Plank_02");
            myPlanks[2] = entity.FindChild("Plank_03");
            myPlanks[3] = entity.FindChild("Plank_04");
            myPlanks[4] = entity.FindChild("Plank_05");

            myAudioSource = entity.GetComponent<AudioSourceComponent>();
            GameManager.Instance.CarpenterEvent += OnCarpenterEvent;
        }

        private void OnUpdate(float deltaTime)
        {
            foreach (Entity plank in myPlanks)
            {
                plank.visible = false;
            }

            for (int i = 0; i < Health; i++)
            {
                myPlanks[i].visible = true;
            }

            if(isInRange && Health < 5 && Input.IsKeyDown(KeyCode.F))
            {
                StartRepair();

                if (myTimeBtwRepairs <= 0)
                {
                    RepairBarricade();
                    myTimeBtwRepairs = 1f;
                }
                else
                {
                    myTimeBtwRepairs -= Time.deltaTime;
                }

                Log.Info("Repair Time: " + myTimeBtwRepairs.ToString());
            }
            else
            {
                myTimeBtwRepairs = 1f;
                StopRepair();
            }
        }
        
        public Vector3 GetBarricadePosition()
        {
            return entity.position;
        }

        public override void ShowUI(bool toggleVisibility)
        {
            if (toggleVisibility)
            {
                //Show appropriate UI elements
                if(Health < 5)
                {
                    UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Repair);
                }
            }
            else if (!toggleVisibility)
            {
                //Get rid of UI element
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
            }
        }

        bool audioStarted;
        uint repairID = 0;
        public void StartRepair()
        {
            if(!audioStarted)
            {
                audioStarted = true;
                repairID = myAudioSource.PlayEvent(WWiseEvents.Play_Barricade_Repair.ToString());
            }
        }
        public void StopRepair()
        {
            if(repairID != 0)
            {
                myAudioSource.StopEvent(repairID);
                audioStarted = false;
            }
        }

    }
}
