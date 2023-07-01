using System.Windows.Forms;
using Volt;
using System.Linq;
using System.Diagnostics;

namespace Project
{
    public class Player : Script
    {
        public float runSpeed = 350f;
        public float sprintSpeed = 472f;
        public float acceleration = 10f;
        public float deceleration = 10f;
        public float jumpForce = 100f;
        public float rotationSpeed = 10f;

        private float currentSpeed = 0f;
        private Vector3 desiredDir;


        CharacterControllerComponent charComp;
        AnimationControllerComponent animController;
        Entity camera;
        private void OnCreate()
        {
        }

        private void OnUpdate(float deltaTime)
        {
            if (camera == null)
            {
                camera = Entity.Find("VCam0");
            }
            charComp = entity.GetComponent<CharacterControllerComponent>();
            animController = entity.GetComponent<AnimationControllerComponent>();
            Vector2 dir = Vector2.Zero;
            if (Volt.Input.IsKeyDown(KeyCode.W))
            {
                dir.y += 1;
            }
            if (Volt.Input.IsKeyDown(KeyCode.S))
            {
                dir.y -= 1;
            }

            if (Volt.Input.IsKeyDown(KeyCode.A))
            {
                dir.x -= 1;
            }
            if (Volt.Input.IsKeyDown(KeyCode.D))
            {
                dir.x += 1;
            }
            float desiredSpeed = runSpeed;
            if (Volt.Input.IsKeyDown(KeyCode.Left_Shift))
            {
                desiredSpeed = sprintSpeed;
            }

            if (dir.Length() > 0)
            {
                currentSpeed = Mathf.Lerp(currentSpeed, desiredSpeed, acceleration * deltaTime);
            }
            else
            {
                currentSpeed = Mathf.Lerp(currentSpeed, 0, deceleration * deltaTime);
            }

            Vector3 cameraFwd = new Vector3(0, 0, 1);
            if (camera != null)
            {
                cameraFwd = camera.forward;
            }
            dir = new Vector2(cameraFwd.x, cameraFwd.z) * dir.y + new Vector2(cameraFwd.z, -cameraFwd.x) * dir.x;
            dir.Normalize();
            if (dir.Length() > 0)
            {
                desiredDir = new Vector3(dir.x, 0, dir.y);

            }
            if (currentSpeed > 0 && charComp != null)
            {

                Vector3 lookDir = new Vector3(dir.x, 0, dir.y);
                Log.Info("LookDir: " + lookDir.ToString());

                entity.rotation = Quaternion.Slerp(entity.rotation, Quaternion.QuaternionLookRotation(desiredDir, Vector3.Down), rotationSpeed * deltaTime);
                charComp.Move(new Vector3(entity.forward.x, 0, entity.forward.z) * currentSpeed * deltaTime);
            }


            animController?.controller?.SetParameter("CurrentSpeed", currentSpeed);
            animController?.controller?.SetParameter("Grounded", charComp.isGrounded);

            animController?.controller?.SetParameter("Jump", false);
            if (Volt.Input.IsKeyDown(KeyCode.Space) && charComp.isGrounded)
            {
                Jump();
            }
        }

        private void Jump()
        {
            if (charComp != null)
            {
                charComp.Jump(jumpForce);
            }
            animController?.controller?.SetParameter("Jump", true);

        }
    }
}
