using Volt;

namespace Project
{
    public class TestPlayer : Script
    {
        private void OnCreate()
        {
            Entity parent = entity.parent;
            parent.position = new Vector3(0f);
        }

        private void OnUpdate(float deltaTime)
        {
            if (Input.IsKeyDown(KeyCode.W))
            {
                entity.position += new Vector3(0f, 0f, 100f) * deltaTime;
            }

            if (Input.IsKeyDown(KeyCode.S))
            {
                entity.position += new Vector3(0f, 0f, -100f) * deltaTime;
            }

            if (Input.IsKeyDown(KeyCode.A))
            {
                entity.position += new Vector3(-100f, 0f, 0f) * deltaTime;
            }

            if (Input.IsKeyDown(KeyCode.D))
            {
                entity.position += new Vector3(100f, 0f, 100f) * deltaTime;
            }
        }
    }
}
