using Volt;

namespace Project
{
    public class TestAiSpawner : Script
    {
        public Prefab ai;

        private void OnCreate()
        {
            
        }

        private void OnUpdate(float deltaTime)
        {
            if (Input.IsKeyPressed(KeyCode.V))
            {
                if (ai != null)
                {
                    NetScene.InstantiatePrefab(ai.handle, entity.Id);
                }
            }
        }
    }
}
