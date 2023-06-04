using System;
using System.Collections.Generic;
using Volt;

namespace Project
{
    public class Weapon : Script
    {
        // Public
        public Prefab MuzzleFlashVFX = null;
        public WeaponBehaviour Behaviour
        {
            get
            {
                return myBehaviour;
            }
            set
            {
                myBehaviour = value;
                myBehaviour.Init(entity);
            }
        }

        // Private
        private Entity myFPSCamera = null;
        private Entity myMuzzle = null;
        private WeaponBehaviour myBehaviour = null;

        private void OnCreate()
        {
            if (entity.parent.HasScript<Player>())
            {
                myFPSCamera = entity.parent.GetScript<Player>().FPSCamera;
            }

            myMuzzle = myFPSCamera.FindChild("PlayerMesh").FindChild("Muzzle");

        }

        private void OnUpdate(float deltaTime)
        {
            Behaviour?.Update(deltaTime);
        }

        private void OnDestroy()
        {
            myBehaviour?.Destroy();
        }

        public bool Shoot(bool isAiming)
        {
            if (Behaviour == null) { return false; }

            if (Behaviour.Shoot(isAiming))
            {
                Behaviour.Recoil?.Apply(isAiming);
                MuzzleFlash();
                return true;
            }

            return false;
        }

        public bool Reload(Action callback = null)
        {
            if (Behaviour == null) { return false; }
            return Behaviour.Reload(callback);
        }

        private void MuzzleFlash()
        {
            if (MuzzleFlashVFX != null)
            {
                if (myFPSCamera != null && myMuzzle != null)
                {
                    myFPSCamera.parent.GetScript<Player>().CharacterAnimationController.controller.AttachEntity("Muzzle", myMuzzle);
                    Entity flash = VFXManager.Instance.SpawnPrefabVFX(MuzzleFlashVFX, .1f);
                    flash.position = myMuzzle.position;
                    flash.rotation = myMuzzle.rotation;

                }
            }
        }
    }

    public enum WeaponMobility
    {
        Low, Medium, High
    }

    public class WeaponInformation
    {
        public string Name = "Unknown";

        public float Damage = 0.0f;
        public float Range = 0.0f;

        public uint ClipAmmo = 0;
        public uint MagazineAmmo = 0;
        public uint MaxClipAmmo = 0;
        public uint MaxMagazineAmmo = 0;

        public float FireRate = 0.0f;
        public float ReloadTime = 0.0f;
        public float EmptyReloadTimeAddon = 0.0f;

        public float HeadMultiplier = 0.0f;
        public float ChestMultiplier = 0.0f;
        public float AbdomenMultiplier = 0.0f;

        public Vector3 Recoil;
        public Vector3 AimRecoil;

        public float Snappiness = 0.0f;
        public float ReturnSpeed = 0.0f;

        public WeaponMobility Mobility = WeaponMobility.Medium;
    }

    public class WeaponBehaviour
    {
        public WeaponId Id = WeaponId.Unknown;

        public bool IsPaP = false;
        public bool IsAutomatic = false;
        public bool IsReloading = false;
        public bool ShouldCancelReload = false;
        public bool IsWaitingForFireRate = false;
        public bool IsFiring = false;

        public WeaponRecoil Recoil = null;

        public WeaponMeta MetaInfo
        {
            get
            {
                return WeaponManager.Instance.GetMetadata(Id);
            }
        }

        public WeaponInformation Information
        {
            get
            {
                if (IsPaP)
                {
                    return myPaPInformation;
                }
                else
                {
                    return myInformation;
                }
            }

            protected set
            {
                if (IsPaP)
                {
                    myPaPInformation = value;
                }
                else
                {
                    myInformation = value;
                }
            }
        }

        protected WeaponInformation myInformation;
        protected WeaponInformation myPaPInformation;

        protected Entity myWeapon;

        #region Public

        public void Destroy()
        {
            if (myWeapon != null && myWeapon.parent != null)
            {
                myWeapon.parent.GetScript<Player>().playerEventHandler.OnPlayerInput -= OnPlayerInputEvent;
            }
            if (GameManager.Instance != null)
            {
                GameManager.Instance.MaxAmmoEvent -= MaxAmmoPowerUp;
            }
        }

        public void Init(Entity weapon)
        {
            myWeapon = weapon;
            myWeapon.parent.GetScript<Player>().playerEventHandler.OnPlayerInput += OnPlayerInputEvent;
            NetEvents.EventFromLocalId(20, eNetEvent.Hit, 300);
            if (GameManager.Instance != null)
            {
                GameManager.Instance.MaxAmmoEvent += MaxAmmoPowerUp;
            }

            myInformation = new WeaponInformation();
            myPaPInformation = new WeaponInformation();

            InitStats();

            Recoil = new WeaponRecoil(myWeapon, myInformation.Recoil, myInformation.AimRecoil, myInformation.Snappiness, myInformation.ReturnSpeed);
        }

        public void Update(float deltaTime)
        {
            Recoil?.Update(deltaTime);
        }

        private void OnPlayerInputEvent(List<PlayerInputType> input)
        {
            if (input.Contains(PlayerInputType.Fire_Released))
            {
                IsFiring = false;
            }
        }

        public void ResetStats()
        {
            InitStats();
        }

        virtual public bool Shoot(bool isAiming)
        {
            if (!IsReadyToShoot()) { return false; }

            Player player = myWeapon.parent.GetScript<Player>();
            Entity camera = player.FPSCamera;
            uint layerMask = 1 << 7;
            layerMask = layerMask | 1 << 3;
            layerMask = layerMask | 1 << 4;

            Vector3 direction = WeaponSpread.GetRandomSpreadDirection(camera.forward, (isAiming) ? 0.025f : 1.5f);

            DebugWeaponTrail debugTrail = new DebugWeaponTrail();
            debugTrail.start = camera.position;
            debugTrail.end = camera.position + direction * Information.Range;
            debugTrail.hit = false;
            debugTrail.lifetime = 5.0f;

            RaycastHit hit;

            if (Physics.Raycast(camera.position, direction, out hit, Information.Range, layerMask))
            {
                float finalDamage = Information.Damage;

                if (hit.entity.HasScript<ColliderAttachment>())
                {
                    debugTrail.hit = true;

                    VFXManager.Instance?.ImpactVFX(ImapctType.Body, hit.position, new Quaternion(hit.normal));

                    ColliderAttachment colliderAttachment = hit.entity.GetScript<ColliderAttachment>();
                    eBodyPart hitPart = colliderAttachment.BodyPart;

                    finalDamage = finalDamage * GetMultiplier(hitPart);

                    finalDamage *= player.myStats.Modifiers.FireRateModifier;

                    NetEvents.EventFromLocalId(hit.entity.Id, eNetEvent.Hit, finalDamage, (byte)hitPart, true);
                }
                else
                {
                    VFXManager.Instance?.ImpactVFX(ImapctType.Wall, hit.position, new Quaternion(hit.normal));
                }
            }

            WeaponManager.Instance.AddDebugWeaponTrail(debugTrail);

            IsFiring = true;
            Information.ClipAmmo -= 1;

            UIManager.Instance.OnWeaponInteraction();

            WaitForFireRate();
            return true;
        }

        virtual public bool Reload(Action callback)
        {
            if (IsReloading || Information.ClipAmmo == Information.MaxClipAmmo || Information.MagazineAmmo <= 0 || (Information.ReloadTime == 0.0f && Information.ClipAmmo > 0)) { return false; }

            IsReloading = true;

            myWeapon.CreateTimer(Information.ReloadTime + ((Information.ClipAmmo == 0) ? Information.EmptyReloadTimeAddon : 0.0f), () =>
            {
                uint bulletsInMag = Information.ClipAmmo;
                uint bulletsToSet = Information.MaxClipAmmo;

                if ((Information.MagazineAmmo + bulletsInMag) < Information.MaxClipAmmo)
                {
                    bulletsToSet = Information.MagazineAmmo;
                }

                Information.ClipAmmo = bulletsToSet;
                Information.MagazineAmmo -= bulletsToSet - bulletsInMag;

                UIManager.Instance.OnWeaponInteraction();

                IsReloading = false;
                Log.Info($"{Information.Name} Reloaded!", "Weapon");

                if (callback != null)
                {
                    callback();
                }
            });

            return true;
        }

        public float GetTimeBetweenShoots()
        {
            return 1.0f / (Information.FireRate / 60.0f);
        }

        public float GetMultiplier(eBodyPart hitPart)
        {
            switch (hitPart)
            {
                case eBodyPart.Head:
                    {
                        return Information.HeadMultiplier;
                    }
                case eBodyPart.Body:
                    {
                        return Information.ChestMultiplier;
                    }
                case eBodyPart.Leg:
                    {
                        return Information.AbdomenMultiplier;
                    }
                default:
                    {
                        return 1.0f;
                    }
            }
        }

        #endregion

        #region Protected

        protected void WaitForFireRate()
        {
            IsWaitingForFireRate = true;

            float timeToWait = GetTimeBetweenShoots();

            myWeapon.CreateTimer(timeToWait, () =>
            {
                IsWaitingForFireRate = false;
                Log.Info($"{Information.Name} is ready to fire!", "Weapon");
            });
        }

        protected bool IsReadyToShoot()
        {
            if (!IsAutomatic && IsFiring)
            {
                return false;
            }
            if (IsReloading)
            {
                return false;
            }
            if (IsWaitingForFireRate)
            {
                return false;
            }
            if (Information.ClipAmmo <= 0)
            {
                return false;
            }
            return true;
        }

        virtual protected void InitStats()
        {

        }

        #endregion

        #region Private

        private void MaxAmmoPowerUp()
        {
            Information.MagazineAmmo = Information.MaxMagazineAmmo;
            Log.Info($"{Information.Name} Max Ammo!", "Weapon");
        }

        #endregion
    }
}
