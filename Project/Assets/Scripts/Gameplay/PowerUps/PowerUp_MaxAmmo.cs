using Volt;

namespace Project
{
    public class PowerUp_MaxAmmo : PowerUp_Base
    {
        private void OnCreate()
        {
            myType = PowerUps.MaxAmmo;
            base.Init();
        }

        private void OnTriggerEnter(Entity other)
        {
            GameManager.Instance.CallMaxAmmoEvent();
            Entity.Destroy(entity);
        }
    }
}