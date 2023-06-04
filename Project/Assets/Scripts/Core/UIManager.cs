using System;
using System.Collections.Generic;
using Volt;
using static Project.GameManager;

namespace Project
{
    public class UIManager : Script 
    {

        #region Static Instance
        static UIManager _instance;
        public static UIManager Instance
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

        public delegate void ChangeUIStateHandler(UIState aState);
        public ChangeUIStateHandler ChangeUIStateEvent;

        bool firstframe = false;

        public enum UIState
        {
            NONE,
            HUD,
            PAUSE,
            GAMEOVER
        }

        bool cursorState = false;
        UIState currentState = UIState.HUD;

        void OnCreate()
        {
            Input.ShowCursor(false);
            currentState = UIState.HUD;
            GameManager.Instance.EndGameEvent += OnEndGame;

        }

        private void OnAwake()
        {
            _instance = this;
        }

        void OnUpdate(float aDeltaTime)
        {
            if (!firstframe)
            {
                firstframe = true;
                ChangeUIStateEvent?.Invoke(currentState);
            }

            if (Input.IsKeyPressed(KeyCode.F1) && currentState != UIState.GAMEOVER)
            {
                cursorState = !cursorState;
                Input.ShowCursor(cursorState);

                if (cursorState)
                {
                    currentState = UIState.PAUSE;
                    ChangeUIStateEvent?.Invoke(currentState);
                }
                else
                {
                    currentState = UIState.HUD;
                    ChangeUIStateEvent?.Invoke(currentState);
                }
            }


            if (Input.IsKeyPressed(KeyCode.F2))
            {
                PointManager.Instance.AddPoints(99999999);
            }

        }

        public delegate void UIEventHandler();

        #region Weapon
        public UIEventHandler WeaponInteractionEvent;
        public void OnWeaponInteraction()
        {
            WeaponInteractionEvent?.Invoke();
        }

        public delegate void CrosshairStateChangeHandler(bool aState);
        public CrosshairStateChangeHandler CrosshairStateChangeEvent;
        public void OnCrosshairStateChange(bool aState)
        {
            CrosshairStateChangeEvent?.Invoke(aState);
        }
        #endregion

        #region Points
        public UIEventHandler PointsInteractionEvent;
        public void OnPointsInteraction()
        {
            PointsInteractionEvent?.Invoke();

        }
        #endregion

        #region Perks
        public UIEventHandler PerkInteractionEvent;
        public void OnPerkInteraction()
        {
            PerkInteractionEvent?.Invoke();
        }
        #endregion

        #region Rounds
        public UIEventHandler RoundsInteractionEvent;
        public void OnRoundsInteraction()
        {
            RoundsInteractionEvent?.Invoke();
        }
        #endregion

        #region Interaction
        public enum InteractionTypes
        {
            Disable = 0,
            NoPower,
            Power,
            NoneCalling,
            Teleport,
            TeleportUsed,
            PackAPunch,
            Repair,
            OpenDoorOne,
            OpenDoorTwo,
            OpenDoorThree,

            BuyPistol,
            BuyAssault,
            BuyBoltAction,
            BuyShotgun,
            
            BuyPerk,
            PerkFireRate,
            PerkMaxHealth,
            PerkReload,
            PerkRevive,
            PerkStamina,

            BuyMysteryWeapon,
            PickUpPistol,
            PickUpAssult,
            PickUpBoltAction,
            PickUpShotgun,
            PickUpThunderGun

        }

        public delegate void PlayerInteractionHandler(InteractionTypes aInteractionType);
        public PlayerInteractionHandler PlayerInteractionEvent;
        public void OnPlayerInteraction(InteractionTypes aInteractionType)
        {
            PlayerInteractionEvent?.Invoke(aInteractionType);
        }
        #endregion

        #region PowerUps

        public UIEventHandler PowerUpInteractionEvent;
        public void OnPowerUpsInteraction()
        {
            PowerUpInteractionEvent?.Invoke();
        }
        #endregion

        #region EndGame
        public UIEventHandler EndGameEvent;
        private void OnEndGame()
        {
            currentState = UIState.GAMEOVER;
            ChangeUIStateEvent?.Invoke(currentState);
            EndGameEvent?.Invoke();
        }

        #endregion

    }
}
