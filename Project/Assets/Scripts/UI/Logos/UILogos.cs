using System.DirectoryServices.Protocols;
using Volt;

namespace Project
{
    public class UILogos : Script
    {
        Sprite TgaLogo;
        Sprite CoinFlipLogo;
        Sprite WwiseLogo;

        public float InitTime = 2;
        public float TimeBetweenLogos = 0;

        void OnCreate()
        {
            TgaLogo = entity.FindChild("Sprite_TGALOGO").GetScript<Sprite>();
            CoinFlipLogo = entity.FindChild("Sprite_Coinflip").GetScript<Sprite>();
            WwiseLogo = entity.FindChild("Sprite_WWISE").GetScript<Sprite>();

            entity.CreateTimer(InitTime, () => { ShowTgaLogo(); });   
        }

        void ShowTgaLogo()
        {
            TgaLogo.entity.visible = true;
            entity.CreateTimer(TimeBetweenLogos, () => { ShowCoinFlipLogo(); });
        }

        void ShowCoinFlipLogo()
        {
            TgaLogo.entity.visible = false;
            CoinFlipLogo.entity.visible = true;
            entity.CreateTimer(TimeBetweenLogos, () => { ShowWwiseLogo(); });
        }

        void ShowWwiseLogo()
        {
            CoinFlipLogo.entity.visible = false;
            WwiseLogo.entity.visible = true;
            entity.CreateTimer(TimeBetweenLogos, () => { VoltApplication.LoadLevel("Assets/Scenes/Levels/SC_LVL_MainMenu/SC_LVL_MainMenu.vtscene"); });   
        }

        void OnUpdate(float aDeltaTime)
        {
            if(Input.IsKeyPressed(KeyCode.Escape) || Input.IsKeyPressed(KeyCode.Space) || Input.IsKeyPressed(KeyCode.Enter))
            {
                VoltApplication.LoadLevel("Assets/Scenes/Levels/SC_LVL_MainMenu/SC_LVL_MainMenu.vtscene");
            }
        }

    }
}
