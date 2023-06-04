using Volt;

namespace Project
{
    public class Thunder : WeaponBehaviour
    {
        public Thunder()
        {

        }

        override protected void InitStats()
        {
            // Information
            Id = WeaponId.Thunder;
            IsAutomatic = false;

            // Non PaP
            myInformation.Name = "Thunder Gun";

            myInformation.Damage = float.MaxValue;
            myInformation.Range = 500.0f;
            myInformation.MaxClipAmmo = 2;
            myInformation.MaxMagazineAmmo = 12;
            myInformation.ClipAmmo = myInformation.MaxClipAmmo;
            myInformation.MagazineAmmo = myInformation.MaxMagazineAmmo;
            myInformation.FireRate = 100;
            myInformation.ReloadTime = 0.0f;
            myInformation.EmptyReloadTimeAddon = 2.0f;

            myInformation.HeadMultiplier = 1.0f;
            myInformation.ChestMultiplier = 1.0f;
            myInformation.AbdomenMultiplier = 1.0f;

            myInformation.Mobility = WeaponMobility.High;

            // PaP
            myPaPInformation.Name = "Crackhead Max";

            myPaPInformation.Damage = float.MaxValue;
            myPaPInformation.Range = 500.0f;
            myPaPInformation.MaxClipAmmo = 4;
            myPaPInformation.MaxMagazineAmmo = 24;
            myPaPInformation.ClipAmmo = myPaPInformation.MaxClipAmmo;
            myPaPInformation.MagazineAmmo = myPaPInformation.MaxMagazineAmmo;
            myPaPInformation.FireRate = 100;
            myPaPInformation.ReloadTime = 0.0f;
            myPaPInformation.EmptyReloadTimeAddon = 2.0f;

            myPaPInformation.HeadMultiplier = 1.0f;
            myPaPInformation.ChestMultiplier = 1.0f;
            myPaPInformation.AbdomenMultiplier = 1.0f;

            myPaPInformation.Mobility = WeaponMobility.High;
        }

        public override bool Shoot(bool isAiming)
        {
            if (IsReloading) { ShouldCancelReload = true; }
            if (!IsReadyToShoot()) { return false; }

            Entity camera = myWeapon.parent.GetScript<Player>().FPSCamera;
            uint layerMask = 1 << 7;

            Vector3 direction = camera.forward;
            float halfSize = Information.Range * 0.5f;

            Entity[] hitEntities = Physics.OverlapBox(camera.position + direction * halfSize, new Vector3(halfSize), layerMask);

            foreach (Entity hitEnt in hitEntities)
            {
                if (hitEnt.HasScript<ColliderAttachment>())
                {
                    NetEvents.EventFromLocalId(hitEnt.Id, eNetEvent.Hit, Information.Damage, (byte)eBodyPart.Other, true);
                }
            }

            IsFiring = true;
            Information.ClipAmmo -= 1;

            UIManager.Instance.OnWeaponInteraction();

            WaitForFireRate();
            return true;
        }
    }
}
