using System;
using Volt;

namespace Project
{
    public class PowerUp_InstaKill : PowerUp_Base
    {
        private void OnCreate()
        {
            myType = PowerUps.InstaKill;
            base.Init();
        }

        private void OnTriggerEnter(Entity other)
        {
            GameManager.Instance.CallInstaKillEvent(true);
            Entity.Destroy(entity);
        }
    }
}
