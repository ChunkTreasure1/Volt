using Volt;
using Volt.AI;

namespace Project
{
    public class TestAi : Script
    {
        public float Speed = 300f;

        int currentIndex = 0;
        Vector3[] path;

        Vector3 start;
        Vector3 goal;

        RigidbodyComponent myRb;

        private void OnCollisionEnter(Entity other)
        {
            Log.Trace("Collided with: " + other.Id.ToString());
        }

        private void OnCreate()
        {
            start = entity.position;
            goal = Entity.Find("Goal").position;

            Vector3 searchDistance = new Vector3(250.0f, 250.0f, 250.0f);

            path = Navigation.FindPath(start, goal, searchDistance);
            foreach (var p in path)
            {
                Log.Trace(p.ToString());
            }

            if (entity.HasComponent<NavAgentComponent>())
            {
                entity.GetComponent<NavAgentComponent>().maxSpeed = Speed;
                entity.GetComponent<NavAgentComponent>().acceleration = Speed * 10.0f;
                entity.GetComponent<NavAgentComponent>().target = goal;
            }
            else if (entity.HasComponent<RigidbodyComponent>())
            {
                myRb = entity.GetComponent<RigidbodyComponent>();
            }
        }

        private void OnUpdate(float deltaTime)
        {
            entity.GetComponent<NavAgentComponent>().target = goal;
            return;

            var agentComp = entity.GetComponent<NavAgentComponent>();

            if (!entity.HasComponent<NavAgentComponent>())
            {
                PathFindUpdate();
            }
            else
            {
                if (Input.IsKeyPressed(KeyCode.Space))
                {
                    agentComp.active = !agentComp.active;
                }

                if (!agentComp.active)
                {
                    if (Input.IsKeyDown(KeyCode.W))
                    {
                        entity.GetComponent<CharacterControllerComponent>().Move(new Vector3(0.0f, 0.0f, 500.0f * deltaTime));
                    }

                    if (Input.IsKeyDown(KeyCode.S))
                    {
                        entity.GetComponent<CharacterControllerComponent>().Move(new Vector3(0.0f, 0.0f, -500.0f * deltaTime));
                    }

                    if (Input.IsKeyDown(KeyCode.A))
                    {
                        entity.GetComponent<CharacterControllerComponent>().Move(new Vector3(-500.0f * deltaTime, 0.0f, 0.0f));
                    }

                    if (Input.IsKeyDown(KeyCode.D))
                    {
                        entity.GetComponent<CharacterControllerComponent>().Move(new Vector3(500.0f * deltaTime, 0.0f, 0.0f));
                    }
                }
            }
        }

        private void PathFindUpdate()
        {
            if (path.Length > 0 && currentIndex < path.Length)
            {
                if (Vector3.Distance(entity.position, path[currentIndex]) < 100f)
                {
                    currentIndex++;
                }

                Vector3 direction = (path[currentIndex] - entity.position).Normalized();
                myRb.linearVelocity = direction * Speed;
            }
        }
    }
}
