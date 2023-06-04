using System.Runtime.Remoting.Messaging;
using System.Web.Http.Controllers;
using Volt;
using Volt.Audio;

namespace Project
{
    public class AudioPlayerHandler : Script
    {
        AudioSourceComponent myAudioSource;
        Player myPlayer;

        void OnCreate()
        {
            myAudioSource = entity.GetComponent<AudioSourceComponent>();
            myPlayer = entity.parent.GetScript<Player>();
        }

        public void PlayFootstep()
        {
            if (myPlayer.IsJumping) return;
            bool isRunning = myPlayer.IsRunning;

            //RAYCAST FIND GROUND MATERIAL

            if (isRunning == true)
            {
                myAudioSource.SetSwitch("Sw_MovementSpeed", "Sprint");
                myAudioSource.PlayEvent(WWiseEvents.Play_Footstep.ToString());
            }
            else
            {
                myAudioSource.SetSwitch("Sw_MovementSpeed", "Run");
                myAudioSource.PlayEvent(WWiseEvents.Play_Footstep.ToString());
            }
        }

        public void PlayJump()
        {
            //RAYCAST FIND GROUND MATERIAL
            myAudioSource.PlayEvent(WWiseEvents.Play_Jump.ToString());
        }
        
        public void PlayLand()
        {
            //RAYCAST FIND GROUND MATERIAL
            myAudioSource.PlayEvent(WWiseEvents.Play_Land.ToString());
        }

        public void PlayMelee(bool isHit)
        {
            if (isHit)
            {
                myAudioSource.SetSwitch("Sw_DamageInflicted", "True");
            }
            else
            {
                myAudioSource.SetSwitch("Sw_DamageInflicted", "False");
            }
            myAudioSource.PlayEvent(WWiseEvents.Play_Melee.ToString());
        }

        public void PlayShoot(WeaponId weaponId, uint magazineAmmoCount)
        {
            //Log.Info(weaponId.ToString() + " / " + magazineAmmoCount.ToString());


            SetAmmoCountSwitch(magazineAmmoCount);

           switch (weaponId) 
            {
                case WeaponId.M1911:
                    myAudioSource.PlayEvent(WWiseEvents.Play_Pistol_Shoot.ToString());
                    return;
                case WeaponId.Galil:
                    myAudioSource.PlayEvent(WWiseEvents.Play_Automatic_Shoot.ToString());
                    return;
                case WeaponId.Spas12:
                    myAudioSource.PlayEvent(WWiseEvents.Play_Shotgun_Shot.ToString());
                    return;
                case WeaponId.Kar98k:
                    myAudioSource.PlayEvent(WWiseEvents.Play_BoltAction_Shoot.ToString());
                    return;
                case WeaponId.Thunder:
                    return;
                default:
                    myAudioSource.PlayEvent(WWiseEvents.Play_Pistol_Shoot.ToString());
                    return;
            }
        }

        void SetAmmoCountSwitch(uint magazineAmmoCount)
        {
            if (magazineAmmoCount == 0)
            {
                myAudioSource.SetSwitch("Sw_AmmoCount", "OutOfAmmo");
            }
            else if (magazineAmmoCount == 1)
            {
                myAudioSource.SetSwitch("Sw_AmmoCount", "LastShot");
            }
            else
            {
                myAudioSource.SetSwitch("Sw_AmmoCount", "NormalShot");
            }
        }

        public void PlayReload(WeaponId weaponId)
        {
            switch (weaponId)
            {
                case WeaponId.M1911:
                    myAudioSource.PlayEvent(WWiseEvents.Play_Pistol_Reload.ToString());
                    return;
                case WeaponId.Galil:
                    myAudioSource.PlayEvent(WWiseEvents.Play_Automatic_Reload.ToString());
                    return;
                case WeaponId.Kar98k:
                    myAudioSource.PlayEvent(WWiseEvents.Play_BoltAction_Reload.ToString());
                    return;
                case WeaponId.Thunder:
                    return;
                default:
                    return;
            }
        }

        public void PlayChangeWeapon(bool isRemove)
        {
            if (isRemove)
            {
                myAudioSource.PlayEvent(WWiseEvents.Play_ChangeWeapon_From.ToString());
            }
            else
            {
                myAudioSource.PlayEvent(WWiseEvents.Play_ChangeWeapon_To.ToString());
            }
        }

        public void TakeDamage(uint playerType)
        {
            //BEEF
            if(playerType == 0)
            {
                myAudioSource.SetSwitch("Sw_PlayerType", "Beef");
                myAudioSource.PlayEvent(WWiseEvents.Play_TakeDamage.ToString());
            }
            //SPEED
            else
            {
                myAudioSource.SetSwitch("Sw_PlayerType", "Speed");
                myAudioSource.PlayEvent(WWiseEvents.Play_TakeDamage.ToString());
            }
        }

        public void Death(uint playerType)
        {
            if (playerType == 0)
            {
                myAudioSource.SetSwitch("Sw_PlayerType", "Beef");
                myAudioSource.PlayEvent(WWiseEvents.Play_Death.ToString());
            }
            //SPEED
            else
            {
                myAudioSource.SetSwitch("Sw_PlayerType", "Speed");
                myAudioSource.PlayEvent(WWiseEvents.Play_Death.ToString());
            }
        }

        public void Equip()
        {
            //myAudioSource.PlayEvent(WWiseEvents.Play_ChangeWeapon_From.ToString());
        }
        public void Unequip()
        {
            //myAudioSource.PlayEvent(WWiseEvents.Play_ChangeWeapon_To.ToString());
        }

        public void Pump()
        {
            myAudioSource.PlayEvent(WWiseEvents.Play_Shotgun_Pumpback.ToString());
        }

        public void ReloadShotgun()
        {
            myAudioSource.PlayEvent(WWiseEvents.Play_Shotgun_Reload.ToString());
        }

        public void Bolt()
        {
            myAudioSource.PlayEvent(WWiseEvents.Play_BoltAction_Bolt.ToString());
        }

    }
}
