using System;
using System.IO;
using System.Security.Cryptography.Xml;
using System.Windows.Forms;
using Volt;
using Volt.AI;

namespace Project
{
    public class EnemyController : Script
    {
        private EnemyStats myStats;

        private Entity myCurrentTarget;
        private Entity[] myPlayers;

        private CharacterControllerComponent myCC;
        private Interactable_Barricade myBarricade = null;

        private bool myBarricadeBroken = false;
        private bool myHasEnteredPlayArea = false;
        private bool isDead = false;

        private float myTimeBtwAttacks;
        private float myUpdateTargetTimer = 0f;
        private float mySpeedMultiplier = 1f;
        private float myCurrentMoveSpeed = 0f;
        private float myResetTimer = 5f;

        private AudioEnemyHandler myAudioHandler;

        private void OnCreate()
        {
            myCC = entity.GetComponent<CharacterControllerComponent>();

            myStats = entity.GetScript<EnemyBase>().GetStats();

            myPlayers = Scene.GetAllEntitiesWithScript<Player>();
            NavAgentComponent agentComp = entity.GetComponent<NavAgentComponent>();

            if (agentComp != null)
            {
                int rand = Volt.Random.Range(0, 2);
                if (rand == 1)
                {
                    NetEvents.EventFromLocalId(entity.Id, eNetEvent.Animation, (byte)eEnemyAnimation.Walk, true);
                }
                else
                {
                    NetEvents.EventFromLocalId(entity.Id, eNetEvent.Animation, (byte)eEnemyAnimation.Run, true);
                }

                agentComp.maxSpeed = rand == 1 ? myStats.WalkMovementSpeed : myStats.RunMovementSpeed;
                myCurrentMoveSpeed = rand == 1 ? myStats.WalkMovementSpeed : myStats.RunMovementSpeed;
                agentComp.acceleration = 1000;
            }

            if (myPlayers.Length >= 1)
            {
                myCurrentTarget = myPlayers[0];
            }

            myAudioHandler = entity.FindChild("Audio").GetScript<AudioEnemyHandler>();

        }

        public void AssignBarricade(Interactable_Barricade barricade)
        {
            myBarricade = barricade;
            myHasEnteredPlayArea = false;

            myBarricadeBroken = false;
            if (myBarricade.IsBroken())
            {
                myBarricadeBroken = true;
            }
        }

        public void LeaveBarricade()
        {
            myHasEnteredPlayArea = true;
            NetEvents.EventFromLocalId(entity.Id, eNetEvent.Animation, (byte)eEnemyAnimation.BitePlank, false);
        }

        private void OnUpdate(float deltaTime)
        {
            if (isDead)
            {
                return;
            }

            Move();

            AttackLogic();

            if (myBarricade != null)
            {
                if (myBarricade.IsBroken())
                {
                    myBarricadeBroken = true;
                }
                else
                {
                    myBarricadeBroken = false;
                }
            }
        }

        private void AttackLogic()
        {
            if (myBarricade != null)
            {
                if (!myHasEnteredPlayArea && !myBarricadeBroken)
                {
                    BarricadeAttackLogic();
                }
                else
                {
                    TargetAttackLogic();
                }
            }
            else
            {
                TargetAttackLogic();
            }
        }

        private void BarricadeAttackLogic()
        {
            Vector3 vecToTarget = myBarricade.GetBarricadePosition() - entity.position;
            float distToTarget = vecToTarget.Length();
            if (distToTarget <= myStats.AttackDistance + 100)
            {
                if (myTimeBtwAttacks <= 0)
                {
                    NetEvents.EventFromLocalId(entity.Id, eNetEvent.Animation, (byte)eEnemyAnimation.BitePlank, true);
                    myBarricade.TakeDamage();
                    myTimeBtwAttacks = myStats.TimeBtwAttacks;
                }
            }
            myTimeBtwAttacks -= Time.deltaTime;
        }

        private void TargetAttackLogic()
        {
            Vector3 vecToTarget = myCurrentTarget.position - entity.position;
            float distToTarget = vecToTarget.Length();
            if (distToTarget <= myStats.AttackDistance + 50)
            {
                if (myTimeBtwAttacks <= 0)
                {
                    NetEvents.EventFromLocalId(entity.Id, eNetEvent.Animation, (byte)eEnemyAnimation.Attack, true);

                    entity.CreateTimer(myStats.TimeBtwAttacks, () => { StopAttack(); });
                    entity.CreateTimer(0.6f, () => { DoAttack(); });

                    myTimeBtwAttacks = myStats.TimeBtwAttacks;
                }
                entity.GetComponent<NavAgentComponent>().active = false;
            }
            myTimeBtwAttacks -= Time.deltaTime;
        }

        private void StopAttack()
        {
            if (entity.HasComponent<NavAgentComponent>())
            {
                entity.GetComponent<NavAgentComponent>().active = true;
            }
        }

        private void DoAttack()
        {
            if (isDead) { return; }

            Vector3 vecToTarget = myCurrentTarget.position - entity.position;

            uint layerMask = 1 << 1;
            Entity[] hitTargets = Physics.OverlapSphere(new Vector3(entity.position.x, entity.position.y + 100, entity.position.z) + entity.forward * (myStats.AttackDistance - 50), 60, layerMask);
            if (hitTargets != null && hitTargets.Length > 0)
            {
                if (hitTargets[0].HasScript<Player>())
                {
                    hitTargets[0].GetScript<Player>().TakeDamage();

                    //ATTACK AUDIO
                    myAudioHandler.PlayAttack(true);
                }
            }
            else
            {
                //ATTACK AUDIO
                myAudioHandler.PlayAttack(false);
            }
        }

        private void Move()
        {
            if (myUpdateTargetTimer <= 0)
            {
                myUpdateTargetTimer = 0.3f;

                if (myPlayers.Length == 2)
                {
                    myCurrentTarget = GetNearestTarget();
                }

                NavAgentComponent navComp = entity.GetComponent<NavAgentComponent>();

                if (myHasEnteredPlayArea || myBarricadeBroken || myBarricade == null)
                {
                    Log.Trace($"Vel: {entity.GetComponent<NavAgentComponent>().velocity.ToString()}");

                    navComp.target = myCurrentTarget.position;
                }
                else
                {
                    navComp.target = myBarricade.GetBarricadePosition();
                }

                float distToTarget = (myCurrentTarget.position - entity.position).Length();
                if (distToTarget > 4000)
                {
                    mySpeedMultiplier = 3;
                }
                else
                {
                    mySpeedMultiplier = 1;
                }

                navComp.maxSpeed = myCurrentMoveSpeed * mySpeedMultiplier;
            }
            else
            {
                myUpdateTargetTimer -= Time.deltaTime;
            }

            //if (myHasEnteredPlayArea || myBarricadeBroken)
            //{
            //    if (entity.GetComponent<NavAgentComponent>().active == true)
            //    {
            //        Vector3 navCompVelocity = entity.GetComponent<NavAgentComponent>().velocity;

            //        if (navCompVelocity == Vector3.Zero)
            //        {
            //            myResetTimer -= Time.deltaTime;

            //            if (myResetTimer <= 0)
            //            {
            //                RetrySpawn();
            //                myResetTimer = 2;
            //            }
            //        }
            //    }
            //}
            //else
            //{
            //    myResetTimer = 2;
            //}
            
            if (myTimeBtwAttacks <= 0)
            {
                Rotate();
            }
            else
            {
                RotateToTarget();
            }
        }

        private void RetrySpawn()
        {
            Log.Info("Reset Enemy");

            Entity[] rooms = Scene.GetAllEntitiesWithScript<Room>();

            int randRoom = Volt.Random.Range(0, rooms.Length);

            entity.position = rooms[randRoom].GetScript<Room>().GetRandomSpawn();

            entity.GetComponent<NavAgentComponent>().target = myCurrentTarget.position;
        }

        private void RotateToTarget()
        {
            Vector3 vecToTarget = (new Vector3(myCurrentTarget.position.x, entity.position.y, myCurrentTarget.position.z) - entity.position).Normalized();
            Quaternion targetRot = Quaternion.QuaternionLookRotation(vecToTarget, -Vector3.Up);
            entity.rotation = Quaternion.Slerp(entity.rotation, targetRot, 1 - (float)Math.Pow(2f, -10 * Time.deltaTime));
        }

        private void Rotate()
        {
            Vector3 moveDir = entity.GetComponent<NavAgentComponent>().velocity;
            
            Quaternion targetRot = Quaternion.QuaternionLookRotation(moveDir, Vector3.Down);
            targetRot.XYZ = new Vector3(0f, targetRot.XYZ.y, 0f);


            Quaternion newRotation = Quaternion.RotateTowards(entity.rotation, targetRot, 500 * Time.deltaTime);

            entity.rotation = newRotation;
        }

        public void OnDeath()
        {
            if (isDead) return;
            GameManager.Instance?.CallEnemyDeathEvent();
            isDead = true;
        }

        private Entity GetNearestTarget()
        {
            if (myPlayers[0].HasScript<Player>())
            {
                if (!myPlayers[0].GetScript<Player>().IsTargetable)
                {
                    return myPlayers[1];
                }

                if (!myPlayers[1].GetScript<Player>().IsTargetable)
                {
                    return myPlayers[0];
                }
            }

            float distToPlayer1 = (myPlayers[0].position - entity.position).Length();
            float distToPlayer2 = (myPlayers[1].position - entity.position).Length();

            return distToPlayer1 < distToPlayer2 ? myPlayers[0] : myPlayers[1];
        }
    }
}
