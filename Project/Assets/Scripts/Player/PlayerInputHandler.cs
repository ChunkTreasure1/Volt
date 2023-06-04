using System.Collections.Generic;
using Volt;

namespace Project
{
    public enum PlayerInputType
    {
        MoveForward,
        MoveForward_Released,
        MoveBackward,
        MoveLeft,
        MoveRight,
        Sprint,
        Sprint_Released,
        Jump,

        SwitchToWeaponOne,
        SwitchToWeaponTwo,

        Fire,
        Fire_Released,
        Aim,
        Aim_Released,

        Melee,

        Reload,
        Size
    }

    public class PlayerInputHandler : Script
    {
        // Delegates
        public delegate void OnPlayerInputEvent(List<PlayerInputType> input);

        // EventHandlers
        public OnPlayerInputEvent OnPlayerInput;

        // Variables
        List<PlayerInputType> inputs = new List<PlayerInputType>();

        Entity myFirstPersonCamera;

        private void OnCreate()
        {
            myFirstPersonCamera = entity.FindChild("FirstPersonCamera");

            if(myFirstPersonCamera != null)
            {
                Vision.SetActiveCamera(myFirstPersonCamera.Id);
            }
        }

        private void OnUpdate(float deltaTime)
        {
            entity.GetScript<Player>().myCharRotY = myFirstPersonCamera.rotation.y;

            if (OnPlayerInput != null)
            {
                inputs.Clear();
                Movement();
                Weapon();

                OnPlayerInput?.Invoke(inputs);
            }
        }

        private void Weapon()
        {
            // Switch to weapon one
            if (Input.IsKeyPressed(KeyCode.Key_0))
            {
                Net.Notify(entity.Id, "myPlayerTestNotificationVariable");

                //NetEvents.EventFromNetId(10, eNetEvent.Hit, 1, 2, 3);
                //Net.Notify(ref netTest);
            }
            if (Input.IsKeyPressed(KeyCode.Key_1))
            {
                inputs.Add(PlayerInputType.SwitchToWeaponOne);
            }

            // Switch to weapon two
            if (Input.IsKeyPressed(KeyCode.Key_2))
            {
                inputs.Add(PlayerInputType.SwitchToWeaponTwo);
            }

            // Fire
            if (Input.IsMouseDown(MouseButton.Left))
            {
                inputs.Add(PlayerInputType.Fire);
            }

            // Stop Fire
            if (Input.IsMouseReleased(MouseButton.Left))
            {
                inputs.Add(PlayerInputType.Fire_Released);
            }

            // Aim
            if (Input.IsMousePressed(MouseButton.Right))
            {
                inputs.Add(PlayerInputType.Aim);
            }

            // Stop Aim
            if (Input.IsMouseReleased(MouseButton.Right))
            {
                inputs.Add(PlayerInputType.Aim_Released);
            }

            // Reload
            if (Input.IsKeyPressed(KeyCode.R))
            {
                inputs.Add(PlayerInputType.Reload);
            }

            // Melee
            if (Input.IsKeyPressed(KeyCode.V) || Input.IsKeyPressed(KeyCode.E) || Input.IsKeyPressed(KeyCode.Q))
            {
                inputs.Add(PlayerInputType.Melee);
            }
        }

        private void Movement()
        {
            // Forward
            if (Input.IsKeyDown(KeyCode.W))
            {
                inputs.Add(PlayerInputType.MoveForward);
            }

            // Stop Forward
            if (Input.IsKeyReleased(KeyCode.W))
            {
                inputs.Add(PlayerInputType.MoveForward_Released);
            }

            // Down
            if (Input.IsKeyDown(KeyCode.S))
            {
                inputs.Add(PlayerInputType.MoveBackward);
            }

            // Left
            if (Input.IsKeyDown(KeyCode.A))
            {
                inputs.Add(PlayerInputType.MoveLeft);
            }

            // Right
            if (Input.IsKeyDown(KeyCode.D))
            {
                inputs.Add(PlayerInputType.MoveRight);
            }

            // Sprint
            if (Input.IsKeyDown(KeyCode.Left_Shift))
            {
                inputs.Add(PlayerInputType.Sprint);
            }

            // Stop Sprint
            if (Input.IsKeyReleased(KeyCode.Left_Shift))
            {
                inputs.Add(PlayerInputType.Sprint_Released);
            }

            // Jump
            if (Input.IsKeyPressed(KeyCode.Space))
            {
                inputs.Add(PlayerInputType.Jump);
            }
        }
    }
}
