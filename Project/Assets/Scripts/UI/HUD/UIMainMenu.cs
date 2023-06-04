using System;
using System.Drawing.Printing;
using System.Dynamic;
using System.IO;
using Volt;
using static Project.MainMenuEnums;

namespace Project
{
    public class MainMenuEnums
    {
        public enum MainMenuState
        {
            START,
            SOLO,
            LOBBY,
            OPTIONS,
            CREDITS
        }
    }

    public class UIMainMenuHandler : Script
    {
        #region Static Instance
        static UIMainMenuHandler _instance;
        public static UIMainMenuHandler Instance
        {
            get
            {
                if (_instance == null)
                {
                    Log.Warning("No Instance available.");
                }
                return _instance;
            }
        }
        #endregion

        public Entity Ent_START;
        public Entity Ent_SOLO;
        public Entity Ent_LOBBY;
        public Entity Ent_OPTIONS;
        public Entity Ent_CREDITS;

        public void ChangeState(MainMenuState aState)
        {
            switch (aState)
            {
                case MainMenuState.START:
                    Ent_START.visible = true;
                    break;
                case MainMenuState.SOLO:
                    Ent_SOLO.visible = true;
                    break;
                case MainMenuState.LOBBY:
                    Ent_LOBBY.visible = true;
                    break;
                case MainMenuState.OPTIONS:
                    Ent_OPTIONS.visible = true;
                    break;
                case MainMenuState.CREDITS:
                    Ent_CREDITS.visible = true;
                    break;
            }
        }

        private void OnAwake()
        {
            _instance = this;
        }
    }

   public class UIMainMenuStart : Script
    {
        public Entity PlaySoloButtonEnt;
        public Entity PlayMultiplayerButtonEnt;
        public Entity OptionsButtonEnt;
        public Entity CreditsButtonEnt;
        public Entity QuitButtonEnt;

        private Button StartSoloButton;
        private Button StartMultiplayerButton;
        private Button OptionsButton;
        private Button CreditsButton;
        private Button QuitButton;

        void OnCreate()
        {
            StartSoloButton = PlaySoloButtonEnt.GetScript<Button>();
            StartMultiplayerButton = PlayMultiplayerButtonEnt.GetScript<Button>();
            OptionsButton = OptionsButtonEnt.GetScript<Button>();
            CreditsButton = CreditsButtonEnt.GetScript<Button>();
            QuitButton = QuitButtonEnt.GetScript<Button>();

            StartSoloButton.OnRelease += OnStartSoloButtonClick;
            StartMultiplayerButton.OnRelease += OnStartLobbyButtonClick;
            OptionsButton.OnRelease += OnOptionsButtonClick;
            CreditsButton.OnRelease += OnCreditsButtonClick;
            QuitButton.OnRelease += OnQuitButtonClick;

            Input.ShowCursor(true);
        }

        void OnEnable()
        {

        }

        void OnStartSoloButtonClick()
        {
            entity.visible = false;
            UIMainMenuHandler.Instance?.ChangeState(MainMenuState.SOLO);
        }

        void OnStartLobbyButtonClick()
        {
            entity.visible = false;
            UIMainMenuHandler.Instance?.ChangeState(MainMenuState.LOBBY);
        }
        void OnOptionsButtonClick()
        {
            entity.visible = false;
            UIMainMenuHandler.Instance?.ChangeState(MainMenuState.OPTIONS);
        }
        void OnCreditsButtonClick()
        {
            entity.visible = false;
            UIMainMenuHandler.Instance?.ChangeState(MainMenuState.CREDITS);
        }

        void OnQuitButtonClick()
        {
            VoltApplication.Quit();
        }

   }

    public abstract class UIMainMenu_Base : Script
    {
        public Entity ReturnButtonEnt;
        protected Button ReturnButton;

        protected void OnBaseCreate()
        {
            ReturnButton = ReturnButtonEnt.GetScript<Button>();
            ReturnButton.OnRelease += OnReturnButton;
        }

        void OnReturnButton()
        {
            entity.visible = false;
            UIMainMenuHandler.Instance.ChangeState(MainMenuState.START);
        }
    }

    public class UIMainMenuSoloPlay : UIMainMenu_Base
    {
        public Entity BeefButtonEnt;
        public Entity SpeedButtonEnt;

        private Button BeefButton;
        private Button SpeedButton;

        void OnCreate()
        {
            base.OnBaseCreate();
            BeefButton = BeefButtonEnt.GetScript<Button>();
            SpeedButton = SpeedButtonEnt.GetScript<Button>();

            BeefButton.OnRelease += OnBeefButton;
            SpeedButton.OnRelease += OnSpeedButton;
        }

        void OnBeefButton()
        {
            using (StreamWriter writer = new StreamWriter(ProjectManager.GetDirectory() + "/Assets/Data/PlayerData.txt"))
            {
                writer.WriteLine(0);
            }
            //PLAY GAME
            string readText = File.ReadAllText(ProjectManager.GetDirectory() + "/Assets/Data/PlayerData.txt");

            //START SERVER
            Net.SetHandleTick(false);
            Net.StartSinglePlayer();
            
            VoltApplication.LoadLevel("Assets/Scenes/Levels/SC_LVL_DerLetzteVonUns/SC_LVL_DerLetzteVonUns.vtscene");
        }

        void OnSpeedButton()
        {
            using (StreamWriter writer = new StreamWriter(ProjectManager.GetDirectory() + "/Assets/Data/PlayerData.txt"))
            {
                writer.WriteLine(1);
            }
            //PLAY GAME
            string readText = File.ReadAllText(ProjectManager.GetDirectory() + "/Assets/Data/PlayerData.txt");

            //START SERVER
            Net.SetHandleTick(false);
            Net.StartSinglePlayer();
            
            VoltApplication.LoadLevel("Assets/Scenes/Levels/SC_LVL_DerLetzteVonUns/SC_LVL_DerLetzteVonUns.vtscene");
        }

    }

    public class UIMainMenuLobby : UIMainMenu_Base
    {
        public Entity CharacterPick;
        public Entity IpInput;

        public Entity BeefButtonEnt;
        public Entity SpeedButtonEnt;

        private Button BeefButton;
        private Button SpeedButton;

        public Entity TextInputEnt;
        private TextInput myTextInput;

        public Entity ConnectButtonEnt;
        private Button ConnectButton;

        void OnCreate()
        {
            base.OnBaseCreate();
            BeefButton = BeefButtonEnt.GetScript<Button>();
            SpeedButton = SpeedButtonEnt.GetScript<Button>();

            BeefButton.OnRelease += OnBeefButton;
            SpeedButton.OnRelease += OnSpeedButton;

            myTextInput = TextInputEnt.GetScript<TextInput>();
            myTextInput.OnInputEnterEvent += OnIpEnter;

            ConnectButton = ConnectButtonEnt.GetScript<Button>();
            ConnectButton.OnRelease += OnIpEnter;
        }

        void OnEnable()
        {
            CharacterPick.visible = true;
            IpInput.visible = false;
        }

        void OnBeefButton()
        {
            using (StreamWriter writer = new StreamWriter(ProjectManager.GetDirectory() + "/Assets/Data/PlayerData.txt"))
            {
                writer.WriteLine(0);
            }

            CharacterPick.visible = false;
            IpInput.visible = true;
        }

        void OnSpeedButton()
        {
            using (StreamWriter writer = new StreamWriter(ProjectManager.GetDirectory() + "/Assets/Data/PlayerData.txt"))
            {
                writer.WriteLine(1);
            }

            CharacterPick.visible = false;
            IpInput.visible = true;
        }

        void OnIpEnter()
        {
            //JOIN GAME

            string s_Ip = myTextInput.Text;
            string[] ips = s_Ip.Split(':');
            string Ip = ips[0];
            string s_port = ips[1];
            int intermediatePort = int.Parse(s_port);
            if(intermediatePort > ushort.MaxValue)
            {
                Log.Error("bad port");
                return;
            }
            ushort port = ushort.Parse(s_port);

            Log.Info(Ip.ToString());
            Log.Info(port.ToString());

            //START CLIENT
            Net.SetHandleTick(false);
            Net.StartClient(port);
            Net.Connect(Ip,port);

            VoltApplication.LoadLevel("Assets/Scenes/Levels/SC_LVL_DerLetzteVonUns/SC_LVL_DerLetzteVonUns.vtscene");
        }
    }

    public class UIMainMenuOptions : UIMainMenu_Base
    {
        private enum eResolution
        {
            p720,
            p1080,
            p1440,
            p2160,
            COUNT

        }
        private enum eWindowMode
        {
            Windowed,
            Fullscreen,
            Borderless,
            COUNT
        }

        Button ResultionIncrease;
        Button ResultionDecrease;
        Text ResolutionText;
        eResolution currentResolution = 0;


        Button WindowModeIncrease;
        Button WindowModeDecrease;
        Text WindowModeText;
        eWindowMode currentWindowMode = 0;

        void OnCreate()
        {
            base.OnBaseCreate();

            ResultionIncrease = entity.FindChild("RESOLUTION").FindChild("Button_Right_Resolution").GetScript<Button>();
            ResultionDecrease = entity.FindChild("RESOLUTION").FindChild("Button_Left_Resolution").GetScript<Button>();
            ResolutionText = entity.FindChild("RESOLUTION").FindChild("Text_ScreenType").GetScript<Text>();

            WindowModeIncrease = entity.FindChild("WINDOWMODE").FindChild("Button_Right_WindowMode").GetScript<Button>();
            WindowModeDecrease = entity.FindChild("WINDOWMODE").FindChild("Button_Left_WindowMode").GetScript<Button>();
            WindowModeText = entity.FindChild("WINDOWMODE").FindChild("Text_WindowMode").GetScript<Text>();

            ResultionIncrease.OnRelease += IncreaseResolution;
            ResultionDecrease.OnRelease += DecreaseResolution;

            WindowModeIncrease.OnRelease += IncreaseWindowMode;
            WindowModeDecrease.OnRelease += DecreaseWindowMode;
        }

        void IncreaseResolution()
        {
            currentResolution++;
            if(currentResolution == eResolution.COUNT)
            {
                currentResolution = 0;
            }
            ChangeResolution();
        }

        void DecreaseResolution()
        {
            currentResolution--;
            if (currentResolution < 0)
            {
                currentResolution = eResolution.p2160;
            }
            ChangeResolution();
        }

        void ChangeResolution()
        {
            switch (currentResolution)
            {
                case eResolution.p720:
                    ResolutionText.TextString = "1280 x 720";
                    VoltApplication.SetResolution(1280, 720);
                    break;
                case eResolution.p1080:
                    ResolutionText.TextString = "1920 x 1080";
                    VoltApplication.SetResolution(1920, 1080);
                    break;
                case eResolution.p1440:
                    ResolutionText.TextString = "2560 x 1440";
                    VoltApplication.SetResolution(2560, 1440);
                    break;
                case eResolution.p2160:
                    ResolutionText.TextString = "3840 x 2160";
                    VoltApplication.SetResolution(3840, 2160);
                    break;
            }
        }

        void IncreaseWindowMode()
        {
            currentWindowMode++;
            if (currentWindowMode == eWindowMode.COUNT)
            {
                currentWindowMode = (eWindowMode)0;
            }

            WindowModeText.TextString = currentWindowMode.ToString();
            VoltApplication.SetWindowMode((uint)currentWindowMode);
        }

        void DecreaseWindowMode()
        {
            currentWindowMode--;
            if (currentWindowMode < 0)
            {
                currentWindowMode = eWindowMode.Borderless;
            }

            WindowModeText.TextString = currentWindowMode.ToString();
            VoltApplication.SetWindowMode((uint)currentWindowMode);
        }
    }

    public class UIMainMenuCredits : UIMainMenu_Base
    {
        void OnCreate()
        {
            base.OnBaseCreate();
        }
    }
}
