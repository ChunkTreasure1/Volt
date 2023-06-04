using Volt;

namespace Project
{
    public class Kar98k : WeaponBehaviour
    {
        public Kar98k()
        {

        }

        override protected void InitStats()
        {
            // Information
            Id = WeaponId.Kar98k;
            IsAutomatic = false;

            // Non PaP
            myInformation.Name = "Bolt Action Rifles";

            myInformation.Damage = 100;
            myInformation.Range = 100000.0f;
            myInformation.MaxClipAmmo = 5;
            myInformation.MaxMagazineAmmo = 50;
            myInformation.ClipAmmo = myInformation.MaxClipAmmo;
            myInformation.MagazineAmmo = myInformation.MaxMagazineAmmo;
            myInformation.FireRate = 48;
            myInformation.ReloadTime = 2.5f;
            myInformation.EmptyReloadTimeAddon = 0.0f;

            myInformation.HeadMultiplier = 3.5f;
            myInformation.ChestMultiplier = 1.7f;
            myInformation.AbdomenMultiplier = 1.7f;

            myInformation.Mobility = WeaponMobility.Low;

            // PaP
            myPaPInformation.Name = "Logga Taiga";

            myPaPInformation.Damage = 200;
            myPaPInformation.Range = 100000.0f;
            myPaPInformation.MaxClipAmmo = 8;
            myPaPInformation.MaxMagazineAmmo = 60;
            myPaPInformation.ClipAmmo = myPaPInformation.MaxClipAmmo;
            myPaPInformation.MagazineAmmo = myPaPInformation.MaxMagazineAmmo;
            myPaPInformation.FireRate = 48;
            myPaPInformation.ReloadTime = 2.5f;
            myPaPInformation.EmptyReloadTimeAddon = 0.0f;

            myPaPInformation.HeadMultiplier = 10.0f;
            myPaPInformation.ChestMultiplier = 6.0f;
            myPaPInformation.AbdomenMultiplier = 1.0f;

            myPaPInformation.Mobility = WeaponMobility.Low;
        }
    }
}
