using Volt;
using Volt.Audio;

namespace Project
{
    public class AudioEnemyHandler : Script
    {
        AudioSourceComponent myAudioSource;

        Entity myPlayer;

        bool growlPlayed = false;
        float growlDistance = 2000f;

        void OnCreate()
        {
            myAudioSource = entity.GetComponent<AudioSourceComponent>();

            Entity[] players = Scene.GetAllEntitiesWithScript<Player>();
            myPlayer = players[0];

        }

        private void OnUpdate(float deltaTime)
        {
            Vector3 vecToTarget = myPlayer.position - entity.position;
            float distToTarget = vecToTarget.Length();

            if(!growlPlayed && distToTarget < growlDistance)
            {
                PlayGrowl();
            }

        }

        public void PlayFootstep()
        {

            //RAYCAST FIND GROUND MATERIAL

            //myAudioSource.SetSwitch("Sw_MovementSpeed", "Sprint");
            //myAudioSource.PlayEvent(WWiseEvents.Play_Enemy_Footsteps.ToString());
        }

        public void PlayAttack(bool isHit)
        {
            if (isHit)
            {
                myAudioSource.SetSwitch("Sw_DamageInflicted", "True");
                myAudioSource.PlayEvent(WWiseEvents.Play_Enemy_Attack.ToString());
            }
            else
            {
                myAudioSource.SetSwitch("Sw_DamageInflicted", "False");
                myAudioSource.PlayEvent(WWiseEvents.Play_Enemy_Attack.ToString());
            }
        }

        public void PlayDeath()
        {
            myAudioSource.PlayEvent(WWiseEvents.Play_Enemy_Death.ToString());
        }

        public void PlayTakeDamage(bool isBullet)
        {
            if (isBullet)
            {
                myAudioSource.SetSwitch("Sw_DamageType", "Bullet");
                myAudioSource.PlayEvent(WWiseEvents.Play_Enemy_TakeDamage.ToString());
            }
            else
            {
                myAudioSource.SetSwitch("Sw_DamageType", "Melee");
                myAudioSource.PlayEvent(WWiseEvents.Play_Enemy_TakeDamage.ToString());
            }
        }

        public void PlayGrowl()
        {
            growlPlayed = true;
            myAudioSource.PlayEvent(WWiseEvents.Play_Enemy_Growl.ToString());
            entity.CreateTimer(10, ResetGrowl);
        }
        private void ResetGrowl()
        {
            growlPlayed = false;
        }

    }
}
