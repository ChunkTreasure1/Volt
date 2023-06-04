using Volt;

namespace Project
{
    public class PowerUp_DoublePoints : PowerUp_Base
    {
        private void OnCreate()
        {
            myType = PowerUps.DoublePoints;
            base.Init();
        }

        private void OnTriggerEnter(Entity other)
        {
            GameManager.Instance.CallDoublePointEvent(true);
            Entity.Destroy(entity);
        }
    }
}
