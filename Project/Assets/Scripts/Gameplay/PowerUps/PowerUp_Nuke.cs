using System;
using System.Reflection.Metadata;
using Volt;

namespace Project
{
    public class PowerUp_Nuke : PowerUp_Base
    {
        private void OnCreate()
        {
            myType = PowerUps.Nuke;
            base.Init();
        }

        private void OnTriggerEnter(Entity other)
        {
            GameManager.Instance.CallNukeEvent();   
            Entity.Destroy(entity);
        }
    }
}