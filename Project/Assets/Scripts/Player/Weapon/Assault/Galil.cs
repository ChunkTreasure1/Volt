using Volt;

namespace Project
{
    public class Galil : WeaponBehaviour
    {
        public Galil()
        {

        }

        override protected void InitStats()
        {
            // Information
            Id = WeaponId.Galil;
            IsAutomatic = true;

            // Non PaP
            myInformation.Name = "Assult Rifle";

            myInformation.Damage = 150;
            myInformation.Range = 100000.0f;
            myInformation.MaxClipAmmo = 35;
            myInformation.MaxMagazineAmmo = 315;
            myInformation.ClipAmmo = myInformation.MaxClipAmmo;
            myInformation.MagazineAmmo = myInformation.MaxMagazineAmmo;
            myInformation.FireRate = 750;
            myInformation.ReloadTime = 2.8f;
            myInformation.EmptyReloadTimeAddon = 1.0f;

            myInformation.HeadMultiplier = 4.0f;
            myInformation.ChestMultiplier = 1f;
            myInformation.AbdomenMultiplier = 1f;

            myInformation.Recoil = new Vector3(-.025f, .0f, 0.0f);
            myInformation.AimRecoil = new Vector3(-0.01f, 0.0f, 0.0f);
            myInformation.Recoil *= 2.0f;
            myInformation.AimRecoil *= 2.0f;

            myInformation.Snappiness = 10.0f;
            myInformation.ReturnSpeed = 5.0f;

            myInformation.Mobility = WeaponMobility.Medium;

            // PaP
            myPaPInformation.Name = "Ivar Point";

            myPaPInformation.Damage = 220;
            myPaPInformation.Range = 100000.0f;
            myPaPInformation.MaxClipAmmo = 35;
            myPaPInformation.MaxMagazineAmmo = 490;
            myPaPInformation.ClipAmmo = myPaPInformation.MaxClipAmmo;
            myPaPInformation.MagazineAmmo = myPaPInformation.MaxMagazineAmmo;
            myPaPInformation.FireRate = 750;
            myPaPInformation.ReloadTime = 2.8f;
            myPaPInformation.EmptyReloadTimeAddon = 1.0f;

            myPaPInformation.HeadMultiplier = 5f;
            myPaPInformation.ChestMultiplier = 1f;
            myPaPInformation.AbdomenMultiplier = 1f;

            myPaPInformation.Recoil = myInformation.Recoil;
            myPaPInformation.AimRecoil = myInformation.AimRecoil;

            myPaPInformation.Snappiness = myInformation.Snappiness;
            myPaPInformation.ReturnSpeed = myInformation.ReturnSpeed;

            myPaPInformation.Mobility = WeaponMobility.Medium;
        }
    }
}
