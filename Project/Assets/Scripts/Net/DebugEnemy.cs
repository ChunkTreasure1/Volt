using Volt;

namespace Project
{
    public class DebugEnemy : Script
    {
        public Prefab prefab;
        public Entity spawnPoint;
        private void OnCreate()
        {
            spawnPoint = Entity.Create();
        }

        private void OnUpdate(float deltaTime)
        {

        }

        public void Hit(int arg1, int arg2, int arg3)
        {
            //NetScene.InstantiatePrefab(prefab.handle, spawnPoint.Id);
            Log.Info("Project.DebugEnemy.Hit called. Args: " + arg1.ToString() + " - " +  arg2.ToString() + " - " + arg3.ToString());
        }

        public void Death()
        {
            NetEvents.EventFromLocalId(entity.Id, eNetEvent.Hit);
        }
    }
}
