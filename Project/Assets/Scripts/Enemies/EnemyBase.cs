using Volt;
using static Project.EnemyController;

namespace Project
{
    public class EnemyBase : Character
    {
        [RepNotify/*("Hit")*/]
        public float notifyTest;
        public float test = 0.0f;

        private EnemyStats myStats;
        private AudioEnemyHandler myAudioHandler;


        private void OnAwake()
        {
            test++;
        }

        private void OnCreate()
        {
            myStats = new EnemyStats();
            myStats.MaxHealth = GameManager.Instance.CurrentRoundHealth;
            myStats.CurrentHealth = myStats.MaxHealth;

            GameManager.Instance.NukeEvent += OnNukeEvent;
            myAudioHandler = entity.FindChild("Audio").GetScript<AudioEnemyHandler>();
        }
        public void SetWaveHealth(float aHealth)
        {
            myStats.MaxHealth = aHealth;
            myStats.CurrentHealth = myStats.MaxHealth;
        }

        private void OnDestroy()
        {
            GameManager.Instance.NukeEvent -= OnNukeEvent;
        }

        private void OnNukeEvent()
        {
            Die();
        }

        public void SetStats(EnemyStats stats)
        {
            myStats = stats;
        }

        public EnemyStats GetStats()
        {
            return myStats;
        }

        public void TakeDamage(float damage, eBodyPart hitPart, bool isFromBullet = true)
        {
            myStats.CurrentHealth -= damage;

            if (GameManager.Instance?.InstaKill == true || myStats.CurrentHealth <= 0)
            {
                myAudioHandler.PlayDeath();
                NetEvents.EventFromLocalId(entity.Id, eNetEvent.Death);

                // Points
                if (isFromBullet)
                {
                    uint killPoints = GetKillPoints(hitPart);
                    if (killPoints > 0)
                    {
                        PointManager.Instance?.AddPoints(killPoints);
                        if (hitPart == eBodyPart.Head)
                        {
                            PointManager.Instance?.AddKill(true);
                        }
                        else
                        {
                            PointManager.Instance?.AddKill(false);
                        }
                    }
                }
                else
                {
                    PointManager.Instance?.AddPoints(130);
                    PointManager.Instance?.AddKill(false);
                }

                return;
            }

            // Points

            PointManager.Instance?.AddPoints(10);

            //AUDIO

            myAudioHandler.PlayTakeDamage(isFromBullet);

            //Log.Info("Enemy Hit!");
        }

        public void Die()
        {
            GameManager.Instance?.SpawnDrops(entity);

            entity.RemoveComponent<CapsuleColliderComponent>();
            entity.RemoveComponent<CharacterControllerComponent>();
            entity.RemoveComponent<NavAgentComponent>();

            foreach (Entity child in entity.children)
            {
                if (child.HasComponent<AnimationControllerComponent>()) { continue; }

                Entity.Destroy(child);
            }
            entity.GetScript<EnemyAnimationHandler>()?.OnDeath();
            var script = entity.GetScript<EnemyController>();
            script?.OnDeath();
        }

        private uint GetKillPoints(eBodyPart hitPart)
        {
            switch (hitPart)
            {
                case eBodyPart.Head:
                    {
                        return 100;
                    }
                case eBodyPart.Body:
                    {
                        return 60;
                    }
                case eBodyPart.Leg:
                    {
                        return 50;
                    }
                default:
                    {
                        return 0;
                    }
            }
        }
    }
}
