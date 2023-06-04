using Volt;
using static Project.UIManager;

namespace Project
{
        public class UIPause : Script
        {
            private Entity MainEnt;
            private Entity OptionsEnt;

            private void OnCreate()
            {
                UIManager.Instance.ChangeUIStateEvent += ChangeUIState;

                MainEnt = entity.FindChild("PAUSE_MAIN");

            }

            public void ChangeUIState(UIManager.UIState aState)
            {
                if (aState == UIState.PAUSE)
                {
                    entity.visible = true;
                }
                else
                {
                    entity.visible = false;
                    GameManager.Instance?.SetPauseMenuState(false);
                }
            }

            void OnEnable()
            {
                MainEnt.visible = true;

                GameManager.Instance?.SetPauseMenuState(true);

                Vision.SetCameraLocked(Vision.GetActiveCamera(), true);

            }

        }

        public class PauseMainHandler : Script
        {
            private Button ResumeButton;
            private Button LeaveButton;

            void OnCreate()
            {
                ResumeButton = entity.FindChild("Button_Resume").GetScript<Button>();
                LeaveButton = entity.FindChild("Button_Leave").GetScript<Button>();

                ResumeButton.OnClick += OnResumeButtonClicked;
                LeaveButton.OnClick += OnLeaveButtonClicked;

            }

            void OnResumeButtonClicked()
            {
                UIManager.Instance.ChangeUIStateEvent(UIState.HUD);
            }

            void OnLeaveButtonClicked()
            {
                VoltApplication.LoadLevel("Assets/Scenes/Levels/SC_LVL_MainMenu/SC_LVL_MainMenu.vtscene");
            }

        }
}
