using System;
using Volt;

namespace Project
{
    public class Spas12 : WeaponBehaviour
    {
        public Spas12()
        {
            
        }

        override protected void InitStats()
        {
            // Information
            Id = WeaponId.Spas12;
            IsAutomatic = false;

            // Non PaP
            myInformation.Name = "Shotgun";

            myInformation.Damage = 160;
            myInformation.Range = 100000.0f;
            myInformation.MaxClipAmmo = 8;
            myInformation.MaxMagazineAmmo = 32;
            myInformation.ClipAmmo = myInformation.MaxClipAmmo;
            myInformation.MagazineAmmo = myInformation.MaxMagazineAmmo;
            myInformation.FireRate = 80;
            myInformation.ReloadTime = 0.567f * myInformation.MaxClipAmmo;
            myInformation.EmptyReloadTimeAddon = 0.0f;

            myInformation.HeadMultiplier = 1.0f;
            myInformation.ChestMultiplier = 1.0f;
            myInformation.AbdomenMultiplier = 1.0f;

            myInformation.Mobility = WeaponMobility.High;

            // PaP
            myPaPInformation.Name = "Animators Love";

            myPaPInformation.Damage = 300;
            myPaPInformation.Range = 100000.0f;
            myPaPInformation.MaxClipAmmo = 24;
            myPaPInformation.MaxMagazineAmmo = 72;
            myPaPInformation.ClipAmmo = myPaPInformation.MaxClipAmmo;
            myPaPInformation.MagazineAmmo = myPaPInformation.MaxMagazineAmmo;
            myPaPInformation.FireRate = 80;
            myPaPInformation.ReloadTime = 1.0f;
            myPaPInformation.EmptyReloadTimeAddon = 0.0f;

            myPaPInformation.HeadMultiplier = 1.0f;
            myPaPInformation.ChestMultiplier = 1.0f;
            myPaPInformation.AbdomenMultiplier = 1.0f;

            myPaPInformation.Mobility = WeaponMobility.High;
        }

        public override bool Shoot(bool isAiming)
        {
            if (IsReloading) { ShouldCancelReload = true; }
            if (!IsReadyToShoot()) { return false; }

            uint pelletsPerShot = 8;

            Player player = myWeapon.parent.GetScript<Player>();
            Entity camera = player.FPSCamera;

            uint layerMask = 1 << 7;
            layerMask = layerMask | 1 << 3;
            layerMask = layerMask | 1 << 4;

            for (int i = 0; i < pelletsPerShot; i++)
            {
                Vector3 direction = WeaponSpread.GetRandomSpreadDirection(camera.forward, 2.5f);

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

                        VFXManager.Instance.ImpactVFX(ImapctType.Body, hit.position, new Quaternion(hit.normal));

                        ColliderAttachment colliderAttachment = hit.entity.GetScript<ColliderAttachment>();
                        eBodyPart hitPart = colliderAttachment.BodyPart;

                        finalDamage = finalDamage * GetMultiplier(hitPart);

                        finalDamage *= player.myStats.Modifiers.FireRateModifier;

                        NetEvents.EventFromLocalId(hit.entity.Id, eNetEvent.Hit, finalDamage, (byte)hitPart, true);
                    }
                    else
                    {
                        VFXManager.Instance.ImpactVFX(ImapctType.Wall, hit.position, new Quaternion(hit.normal));
                    }
                }

                WeaponManager.Instance.AddDebugWeaponTrail(debugTrail);
            }

            IsFiring = true;
            Information.ClipAmmo -= 1;

            UIManager.Instance.OnWeaponInteraction();

            WaitForFireRate();
            return true;
        }

        public override bool Reload(Action callback)
        {
            if (IsReloading || Information.ClipAmmo == Information.MaxClipAmmo || Information.MagazineAmmo <= 0) { return false; }

            if (!IsPaP)
            {
                IsReloading = true;
                ShotgunReloadRecursive(callback);
            }
            else
            {
                base.Reload(callback);
            }

            return true;
        }

        private void ShotgunReloadRecursive(Action callback)
        {
            myWeapon.CreateTimer(Information.ReloadTime / Information.MaxClipAmmo, () =>
            {
                Information.ClipAmmo += 1;
                Information.MagazineAmmo -= 1;

                UIManager.Instance.OnWeaponInteraction();

                if (!ShouldCancelReload && Information.ClipAmmo < Information.MaxClipAmmo && Information.MagazineAmmo > 0)
                {
                    ShotgunReloadRecursive(callback);
                }
                else
                {
                    IsReloading = false;
                    ShouldCancelReload = false;

                    if (callback != null)
                    {
                        callback();
                    }
                }

                Log.Info($"{Information.Name} Reloaded!", "Weapon");
            });
        }
    }
}
