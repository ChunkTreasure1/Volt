using Volt;

namespace Project
{
    public class SpawnArea : Script
    {
        private void OnTriggerExit(Entity other)
        {
            if (other.HasScript<EnemyController>())
            {
                other.GetScript<EnemyController>().LeaveBarricade();
            }
        }

        private void OnTriggerEnter(Entity other)
        {
            if (other.HasScript<EnemyController>())
            {
                Entity barricade = new Entity();
                foreach (Entity ent in entity.children)
                {
                    if (ent.HasScript<Interactable_Barricade>())
                    {
                        barricade = ent;
                    }
                }

                Interactable_Barricade script = barricade.GetScript<Interactable_Barricade>();

                if (script != null)
                {
                    other.GetScript<EnemyController>().AssignBarricade(script);
                }
                else
                {
                    other.GetScript<EnemyController>().AssignBarricade(null);
                }
            }
        }
    }
}
