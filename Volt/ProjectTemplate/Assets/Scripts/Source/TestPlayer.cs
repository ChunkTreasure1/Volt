using System;
using Volt;

namespace ProjectTemplate
{
    public class TestPlayer : Entity
    {
        public bool b = true;
        public Entity entity;
        public float Force = 1000f;

        private TransformComponent myTransformComponent;
        private RigidbodyComponent myRigidbodyComponent;

        private void OnCreate()
        {
            myTransformComponent = GetComponent<TransformComponent>();
            myRigidbodyComponent = GetComponent<RigidbodyComponent>();
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
                myRigidbodyComponent.AddForce(Vector3.Up * 100f, ForceMode.Force);
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

            if (Input.IsKeyDown(KeyCode.Space))
            {
                myRigidbodyComponent.AddForce(Vector3.Up * Force, ForceMode.Force);
            }

            myTransformComponent.position = currTrans;
        }
    }
}
