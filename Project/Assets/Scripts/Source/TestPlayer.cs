using System;
using Volt;

namespace ProjectTemplate
{
    public class TestPlayer : Entity
    {
        public bool b = true;
        public Entity entity;

        private TransformComponent myTransformComponent;

        private void OnCreate()
        {
            myTransformComponent = GetComponent<TransformComponent>();
        }

        private void OnUpdate(float deltaTime)
        { 
            const float speed = 100f;

            Vector3 currTrans = myTransformComponent.position;

            if (Input.IsKeyDown(KeyCode.W))
            {
                currTrans.z += speed * deltaTime;
            }

            if (Input.IsKeyDown(KeyCode.S))
            {
                currTrans.z -= speed * deltaTime;
            }

            if (Input.IsKeyDown(KeyCode.A))
            {
                currTrans.x -= speed * deltaTime;
            }

            if (Input.IsKeyDown(KeyCode.D))
            {
                currTrans.x += speed * deltaTime;
            }

            myTransformComponent.position = currTrans;
        }
    }
}
