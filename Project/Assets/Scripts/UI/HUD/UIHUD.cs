using System.Collections.Generic;

using Volt;
using static Project.UIManager;

namespace Project
{
    public class UIHUD : Script
    {
        private Entity WeaponInfo;
        private Entity PointsInfo;
        private Entity PerksInfo;
        private Entity RoundsInfo;
        private Entity InteractInfo;
        private Entity CrosshairInfo;
        private Entity PowerUpsInfo;


        private void OnCreate()
        {
            UIManager.Instance.ChangeUIStateEvent += ChangeHUDState;

            WeaponInfo = entity.FindChild("HUD_WEAPON");
            PointsInfo = entity.FindChild("HUD_POINTS");
            PerksInfo = entity.FindChild("HUD_PERK");
            RoundsInfo = entity.FindChild("HUD_ROUNDS");
            InteractInfo = entity.FindChild("HUD_INTERACT");
            CrosshairInfo = entity.FindChild("HUD_CROSSHAIR");
            PowerUpsInfo = entity.FindChild("HUD_POWERUPS");

            ResetHUD();
        }

        void ChangeHUDState(UIManager.UIState aState)
        {
            if (aState == UIState.HUD)
            {
                entity.visible = true;
                ResetHUD();
            }
            else
            {
                TurnOffHUD();
                entity.visible = false;
            }
        }

        private void OnEnable()
        {
            ResetHUD();
        }

        private void ResetHUD()
        {
            WeaponInfo.visible = true;
            PointsInfo.visible = true;
            PerksInfo.visible = false;
            RoundsInfo.visible = true;
            CrosshairInfo.visible = true;
            InteractInfo.visible = false;
            PowerUpsInfo.visible = false;

            Vision.SetCameraLocked(Vision.GetActiveCamera(), false);
            Input.ShowCursor(false);
        }

        void TurnOffHUD()
        {
            WeaponInfo.visible = false;
            PointsInfo.visible = false;
            PerksInfo.visible = false;
            RoundsInfo.visible = false;
            CrosshairInfo.visible = false;
            InteractInfo.visible = false;
            PowerUpsInfo.visible = false;
        }

    }

    public class HUDPerkHandler : Script
    {
        private Sprite PerkSlot1;
        private Sprite PerkSlot2;
        private Sprite PerkSlot3;
        private Sprite PerkSlot4;

        public Texture FireRate;
        public Texture MaxHealth;
        public Texture Reload;
        public Texture Revive;
        public Texture Stamina;

        private List<Sprite> myPerkSlots;

        void OnCreate()
        {
            PerkSlot1 = entity.FindChild("PerkSlot1").GetScript<Sprite>();
            PerkSlot1.entity.visible = false;
            PerkSlot2 = entity.FindChild("PerkSlot2").GetScript<Sprite>();
            PerkSlot2.entity.visible = false;
            PerkSlot3 = entity.FindChild("PerkSlot3").GetScript<Sprite>();
            PerkSlot3.entity.visible = false;
            PerkSlot4 = entity.FindChild("PerkSlot4").GetScript<Sprite>();
            PerkSlot4.entity.visible = false;

            myPerkSlots = new List<Sprite>();

            myPerkSlots.Add(PerkSlot1);
            myPerkSlots.Add(PerkSlot2);
            myPerkSlots.Add(PerkSlot3);
            myPerkSlots.Add(PerkSlot4);

            UIManager.Instance.PerkInteractionEvent += OnPerkInteraction;

        }

        void OnEnable()
        {
            OnPerkInteraction();
        }

        void OnPerkInteraction()
        {
            PerkSlot1.entity.visible = false;
            PerkSlot2.entity.visible = false;
            PerkSlot3.entity.visible = false;
            PerkSlot4.entity.visible = false;

            List<Perk> perkList = PerkManager.Instance.PlayerPerks;

            for (int i = 0; i < perkList.Count; i++)
            {
                myPerkSlots[i].Texture = GetAssetPath(perkList[i]);
                myPerkSlots[i].entity.visible = true;
            }
        }

        Texture GetAssetPath(Perk aPerk)
        {
            switch (aPerk)
            {
                case Perk.FireRate:
                    return FireRate;
                case Perk.MaxHealth:
                    return MaxHealth;
                case Perk.Reload:
                    return Reload;
                case Perk.Revive:
                    return Revive;
                case Perk.Stamina:
                    return Stamina;
            }

            return FireRate;
        }
    }

    public class HUDPointsHandler : Script
    {
        public Prefab PointGainText;

        private Text PointsText;
        uint currentPoints;

        void OnCreate()
        {
            PointsText = entity.FindChild("PointsText").GetScript<Text>();
            currentPoints = PointManager.Instance.currentPoints;
            PointsText.TextString = currentPoints.ToString();
            UIManager.Instance.PointsInteractionEvent += PointsChangedEvent;
        }

        void PointsChangedEvent()
        {
            if (PointGainText != null)
            {
                int pointGain = (int)PointManager.Instance.currentPoints - (int)currentPoints;

                Entity text = Entity.Create(PointGainText);
                if (text.HasScript<Text>())
                {
                    var textScript = text.GetScript<Text>();

                    if(pointGain > 0)
                    {
                        textScript.TextString = "+" + pointGain.ToString();
                        textScript.TextColor = new Color(1, 1, 0, 0.8f);
                    }
                    else
                    {
                        textScript.TextString = pointGain.ToString();
                        textScript.TextColor = new Color(1, 0, 0, 0.8f);
                    }
                }
                text.position = new Vector3(PointsText.entity.position.x + 30, PointsText.entity.position.y + 50, 1);
            }

            currentPoints = PointManager.Instance.currentPoints;
            PointsText.TextString = currentPoints.ToString();
        }
    }

    public class HUDWeaponHandler : Script
    {
        private Text AmmoText;
        private Text WeaponText;

        private WeaponInformation currentWeaponInfo;

        void OnCreate()
        {
            AmmoText = entity.FindChild("AmmoText").GetScript<Text>();
            WeaponText = entity.FindChild("WeaponText").GetScript<Text>();

            UIManager.Instance.WeaponInteractionEvent += OnWeaponInteraction;
        }

        void OnWeaponInteraction()
        {
            Entity[] players = Scene.GetAllEntitiesWithScript<PlayerInputHandler>();
            currentWeaponInfo = players[0].GetScript<Player>().GetCurrentWeapon().Information;

            WeaponText.TextString = currentWeaponInfo.Name.ToString();
            AmmoText.TextString = currentWeaponInfo.ClipAmmo.ToString() + " / " + currentWeaponInfo.MagazineAmmo.ToString();
        }
    }

    public class HUDRoundsHandler : Script
    {
        private Text RoundsText;
        bool pingpong = false;
        float color = 0;
        uint count = 0;
        float textSize;

        void OnCreate()
        {
            RoundsText = entity.FindChild("RoundsText").GetScript<Text>();
            if (GameManager.Instance != null)
            {
                GameManager.Instance.NewRoundEvent += NewRoundEvent;
            }
            textSize = RoundsText.TextSize.x;

        }

        private void OnUpdate(float aDeltaTime)
        {
            if (!pingpong) { return; }
            color = PingPongLerp(color, 0, 1);
            color += aDeltaTime * 0.75f ;

            if (color >= 1)
            {
                count++;
                color = 0;
            }

            if (count == 5)
            {
                pingpong = false;
                color = 1;
            }
            RoundsText.TextColor = new Color(1, color, color, 0.8f);

        }

        float PingPongLerp(float t, float min, float max)
        {
            float range = max - min;
            float value = t % (range * 2.0f);

            if (value > range)
                value = (range * 2.0f) - value;

            return value + min;
        }

        void NewRoundEvent()
        {
            uint currentRound = GameManager.Instance.GetCurrentRound();
            RoundsText.TextString = currentRound.ToString();
            pingpong = true;
            color = 0;
            count = 0;
        }



    }

    public class HUDInteractHandler : Script
    {
        private Text InteractText;

        void OnCreate()
        {
            InteractText = entity.FindChild("InteractText").GetScript<Text>();
            InteractText.entity.visible = false;
            UIManager.Instance.PlayerInteractionEvent += ShowInteractText;
        }

        void ShowInteractText(UIManager.InteractionTypes aInteractType)
        {
            if (aInteractType == InteractionTypes.Disable)
            {
                InteractText.entity.visible = false;
                return;
            }
            else
            {
                InteractText.entity.visible = true;
            }

            switch (aInteractType)
            {
                case InteractionTypes.NoPower:
                    InteractText.TextString = "There is no power";
                    break;
                case InteractionTypes.Power:
                    InteractText.TextString = "Hold F to activate the power";
                    break;
                case InteractionTypes.Teleport:
                    InteractText.TextString = "Hold F to answer the phone";
                    break;
                case InteractionTypes.TeleportUsed:
                    InteractText.TextString = "Phone Booth On Cooldown";
                    break;
                case InteractionTypes.NoneCalling:
                    InteractText.TextString = "No one is calling";
                    break;
                case InteractionTypes.PackAPunch:
                    InteractText.TextString = "Hold F to upgrade your weapon. Cost 5000";
                    break;
                case InteractionTypes.Repair:
                    InteractText.TextString = "Hold F to repair barricade";
                    break;
                case InteractionTypes.OpenDoorOne:
                    InteractText.TextString = "Hold F to open door. Cost 500";
                    break;
                case InteractionTypes.OpenDoorTwo:
                    InteractText.TextString = "Hold F to open door. Cost 750";
                    break;
                case InteractionTypes.OpenDoorThree:
                    InteractText.TextString = "Hold F to open door. Cost 1000";
                    break;

                case InteractionTypes.BuyPistol:
                    InteractText.TextString = "Hold F to buy the Pistol. Cost 500";
                    break;
                case InteractionTypes.BuyAssault:
                    InteractText.TextString = "Hold F to buy the Assault Rifle. Cost 1250";
                    break;
                case InteractionTypes.BuyBoltAction:
                    InteractText.TextString = "Hold F to buy the Bolt Action Rifle. Cost 1000";
                    break;
                case InteractionTypes.BuyShotgun:
                    InteractText.TextString = "Hold F to buy the Shotgun. Cost 1000";
                    break;

                case InteractionTypes.BuyPerk:
                    InteractText.TextString = "Hold F to randomize a Perk. Cost 1500";
                    break;
                case InteractionTypes.PerkFireRate:
                    InteractText.TextString = "Hold F to pick up Fire Rate Up";
                    break;
                case InteractionTypes.PerkMaxHealth:
                    InteractText.TextString = "Hold F to pick up Max Health Up";
                    break;
                case InteractionTypes.PerkReload:
                    InteractText.TextString = "Hold F to pick up Reload Speed Up";
                    break;
                case InteractionTypes.PerkRevive:
                    InteractText.TextString = "Hold F to pick up Quick Revive";
                    break;
                case InteractionTypes.PerkStamina:
                    InteractText.TextString = "Hold F to pick up Stamina Up";
                    break;

                case InteractionTypes.BuyMysteryWeapon:
                    InteractText.TextString = "Hold F to randomize a weapon. Cost 950";
                    break;
                case InteractionTypes.PickUpPistol:
                    InteractText.TextString = "Hold F to pick up the Pistol";
                    break;
                case InteractionTypes.PickUpAssult:
                    InteractText.TextString = "Hold F to pick up the Assault Rifle";
                    break;
                case InteractionTypes.PickUpBoltAction:
                    InteractText.TextString = "Hold F to pick up the Bolt Action Rifle";
                    break;
                case InteractionTypes.PickUpShotgun:
                    InteractText.TextString = "Hold F to pick up the Shotgun";
                    break;
                case InteractionTypes.PickUpThunderGun:
                    InteractText.TextString = "Hold F to pick up the Thundergun";
                    break;
            }
        }
    }

    public class HUDCrosshairHandler : Script
    {
        Entity CrosshairParent;

        void OnCreate()
        {
            CrosshairParent = entity.FindChild("CrosshairParent");
            UIManager.Instance.CrosshairStateChangeEvent += ChangeAimState;
        }

        void ChangeAimState(bool aState)
        {
            CrosshairParent.visible = !aState;
        }

    }

    public class HUDPowerupsHandler : Script
    {
        Sprite Slot1;
        Sprite Slot2;

        void OnCreate()
        {
            Slot1 = entity.FindChild("SLOT1").GetScript<Sprite>();
            Slot1.entity.visible = false;
            Slot2 = entity.FindChild("SLOT2").GetScript<Sprite>();
            Slot2.entity.visible = false;

            UIManager.Instance.PowerUpInteractionEvent += OnPowerUpInteraction;
        }

        void OnEnable()
        {
            OnPowerUpInteraction();
        }

        void OnPowerUpInteraction()
        {
            bool doublePoints = GameManager.Instance.DoublePoints;
            bool instaKill = GameManager.Instance.InstaKill;

            Slot1.entity.visible = doublePoints;
            Slot2.entity.visible = instaKill;
        }
    }
}
