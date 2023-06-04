using Volt;

namespace Project
{
    public class M1911 : WeaponBehaviour
    {
        public M1911()
        {

        }

        override protected void InitStats()
        {
            // Information
            Id = WeaponId.M1911;
            IsAutomatic = false;

            // Non PaP
            myInformation.Name = "Pistol";

            myInformation.Damage = 20;
            myInformation.Range = 100000.0f;
            myInformation.MaxClipAmmo = 8;
            myInformation.MaxMagazineAmmo = 80;
            myInformation.ClipAmmo = myInformation.MaxClipAmmo;
            myInformation.MagazineAmmo = myInformation.MaxMagazineAmmo;
            myInformation.FireRate = 625;
            myInformation.ReloadTime = 1.63f;
            myInformation.EmptyReloadTimeAddon = 0.17f;

            myInformation.HeadMultiplier = 3.5f;
            myInformation.ChestMultiplier = 1.25f;
            myInformation.AbdomenMultiplier = 1.1f;

            myInformation.Recoil = new Vector3(-.05f, .0f, 0.0f);
            myInformation.AimRecoil = new Vector3(-0.025f, 0.0f, 0.0f);

            myInformation.Snappiness = 10.0f;
            myInformation.ReturnSpeed = 5.0f;

            myInformation.Mobility = WeaponMobility.High;

            // PaP
            myPaPInformation.Name = "Skill Issue";

            myPaPInformation.Damage = 2200;
            myPaPInformation.Range = 100000.0f;
            myPaPInformation.MaxClipAmmo = 12;
            myPaPInformation.MaxMagazineAmmo = 80;
            myPaPInformation.ClipAmmo = myPaPInformation.MaxClipAmmo;
            myPaPInformation.MagazineAmmo = myPaPInformation.MaxMagazineAmmo;
            myPaPInformation.FireRate = 937;
            myPaPInformation.ReloadTime = 1.6f;
            myPaPInformation.EmptyReloadTimeAddon = 0.0f;

            myPaPInformation.HeadMultiplier = 3.5f;
            myPaPInformation.ChestMultiplier = 1.25f;
            myPaPInformation.AbdomenMultiplier = 1.1f;

            myPaPInformation.Recoil = myInformation.Recoil;
            myPaPInformation.AimRecoil = myInformation.AimRecoil;

            myPaPInformation.Snappiness = myInformation.Snappiness;
            myPaPInformation.ReturnSpeed = myInformation.ReturnSpeed;

            myPaPInformation.Mobility = WeaponMobility.High;
        }
    }
}
