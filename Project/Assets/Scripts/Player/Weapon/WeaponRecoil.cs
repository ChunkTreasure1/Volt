using Volt;

namespace Project
{
    public class WeaponRecoil
    {
        private Entity myWeapon;

        private Quaternion myCurrentRotation;
        private Quaternion myTargetRotation;

        private Vector3 myRecoil;
        private Vector3 myAimRecoil;

        private float mySnapiness;
        private float myReturnSpeed;

        public WeaponRecoil(Entity weapon, Vector3 recoil, Vector3 aimRecoil, float snapiness, float returnSpeed)
        {
            myWeapon = weapon;

            myRecoil = recoil;  
            myAimRecoil = aimRecoil;
            mySnapiness = snapiness;
            myReturnSpeed = returnSpeed;
        }

        public void Update(float deltaTime)
        {
            if (myWeapon.HasScript<Weapon>())
            {
                Weapon weapon = myWeapon.GetScript<Weapon>();

                myTargetRotation = Quaternion.Slerp(myTargetRotation, new Quaternion(Vector3.Zero), myReturnSpeed * deltaTime);
                myCurrentRotation = Quaternion.Slerp(myCurrentRotation, myTargetRotation, mySnapiness * deltaTime);

                weapon.entity.localRotation = myCurrentRotation;

                weapon.entity.parent.GetScript<Player>().FPSCamera.rotation = weapon.entity.parent.GetScript<Player>().FPSCamera.localRotation * weapon.entity.localRotation;
            }
        }

        public void Apply(bool isAiming)
        {
            if (myWeapon.HasScript<Weapon>() && isAiming)
            {
                myTargetRotation = new Quaternion(myTargetRotation.XYZ + new Vector3(myAimRecoil.x, Random.Range(-myAimRecoil.y, myAimRecoil.y), Random.Range(-myAimRecoil.z, myAimRecoil.z)));
            }
            else
            {
                myTargetRotation = new Quaternion(myTargetRotation.XYZ + new Vector3(myRecoil.x, Random.Range(-myRecoil.y, myRecoil.y), Random.Range(-myRecoil.z, myRecoil.z)));
            }
        }
    }
}
