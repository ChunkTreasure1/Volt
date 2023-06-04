using System.Data.Metadata.Edm;
using Volt;

namespace Project
{
    public class WeaponSway : Script
    {
        Quaternion myTargetRotation = new Quaternion(Vector3.Zero);
        Quaternion myCurrentRotation = new Quaternion(Vector3.Zero);

        Vector3 myBasePos;
        Vector3 myTargetPosition;
        Vector3 myCurrentPosition;

        float mySwaySpeed = 5.0f;
        float myBounceSpeed = 5.0f;

        private void OnCreate()
        {
            myBasePos = entity.children[0].localPosition;
        }

        void OnUpdate(float deltaTime)
        {
            // Sway
            foreach (Entity child in entity.children)
            {
                myTargetRotation = Quaternion.Slerp(myTargetRotation, new Quaternion(Vector3.Zero), mySwaySpeed * deltaTime);
                myCurrentRotation = Quaternion.Slerp(myCurrentRotation, myTargetRotation, mySwaySpeed * deltaTime);

                child.localRotation = myCurrentRotation;
            }

            //Bounce && Side-sway
            foreach (Entity child in entity.children)
            {
                myTargetPosition = Vector3.Lerp(myTargetPosition, myBasePos, myBounceSpeed * deltaTime);
                myCurrentPosition = Vector3.Lerp(myCurrentPosition, myTargetPosition, myBounceSpeed * deltaTime);

                child.localPosition = myCurrentPosition;
            }
        }

        public void SetTargetRotation(Vector3 targetRotation)
        {
            myTargetRotation = new Quaternion(targetRotation);
        }

        public void SetTargetBounce(float bounceAmount)
        {
            myTargetPosition = myBasePos - new Vector3(0, bounceAmount, 0);
        }

        public void SetTargetSideSway(float swayAmount)
        {
            myTargetPosition = myBasePos - new Vector3(swayAmount, 0, 0);
        }
    }
}
