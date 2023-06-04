using Volt;

namespace Project
{
    public class PowerUp_Carpenter : PowerUp_Base
    {
        private void OnCreate()
        {
            myType = PowerUps.Carpenter;
            base.Init();
        }

        private void OnTriggerEnter(Entity other)
        {
            GameManager.Instance.CallCarpenterEvent();
            Entity.Destroy(entity);
        }
    }
}