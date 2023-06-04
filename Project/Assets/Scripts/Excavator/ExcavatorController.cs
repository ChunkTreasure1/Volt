using Volt;

namespace Project
{
    public class ExcavatorController : Script
    {
        public Entity PlayerPositionTarget;

        private bool myHasControl = false;
        private Entity myPlayerEntity;

        public void Enter(Entity player)
        {
            myPlayerEntity = player;
            myHasControl = true;
        }

        private void OnCreate()
        {
        }

        private void OnUpdate(float deltaTime)
        {
            if (!myHasControl) { return; }

            const float speed = 500f;

            if (Input.IsKeyDown(KeyCode.W)) 
            {
                Vector3 vel = -new Vector3(entity.up.x, 0f, entity.up.z) * speed;
                entity.GetComponent<RigidbodyComponent>().SetLinearVelocity(vel);
            }

            if (Input.IsKeyDown(KeyCode.S))
            {
                Vector3 vel = new Vector3(entity.up.x, 0f, entity.up.z) * speed;
                entity.GetComponent<RigidbodyComponent>().SetLinearVelocity(vel);
            }

            if (Input.IsKeyDown (KeyCode.D))
            {
                entity.GetComponent<RigidbodyComponent>().SetAngularVelocity(new Vector3(0f, 1, 0f));
            }

            if (Input.IsKeyDown(KeyCode.A))
            {
                entity.GetComponent<RigidbodyComponent>().SetAngularVelocity(new Vector3(0f, -1, 0f));
            }

            myPlayerEntity.position = PlayerPositionTarget.position;

        }
    }
}
