using Volt;

namespace Project
{
    public class Interactable_PhoneBooth : Interactable_Base
    {
        static public float TimeUntilReturn
        {
            get
            {
                return myTimeUntilReturn;
            }

            set
            {
                myTimeUntilReturn = value;
            }
        }
        static private float myTimeUntilReturn = 30;

        Entity[] teleportPosition;
        Entity[] returnPosition;

        bool myIsActivated = false;
        bool myIsOnCooldown = false;

        public override void InteractEvent()
        {
            if (myIsActivated && !myIsOnCooldown)
            {
                teleportPosition = Scene.GetAllEntitiesWithScript<TeleportPosition>();
                Entity[] players = Scene.GetAllEntitiesWithScript<PlayerInputHandler>();

                if (players.Length > 0)
                {
                    players[0].GetScript<Player>().IsTargetable = false;
                    BrokenLamp brokenLamp = entity.GetScriptInChildren<BrokenLamp>();

                    entity.CreateTimer(2, () => 
                    {
                        players[0].position = teleportPosition[0].position;

                        if (brokenLamp != null)
                        {
                            brokenLamp.IsCrazy = false;
                        }
                    });

                    UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);

                    GameManager.Instance.ResetPhoneBoothEvent += ResetPlayer;
                    GameManager.Instance.NewRoundEvent += ReactivateAfterRound;

                    GameManager.Instance.CallPhoneBoothEvent();

                    if (brokenLamp != null)
                    {
                        brokenLamp.IsCrazy = true;
                    }

                    VFXManager.Instance.StartBlinkEffectPPFX();

                    myIsOnCooldown = true;

                    entity.FindChild("Audio").GetScript<AudioPhoneBooth>().CancelRinging();
                }
            }
        }

        public void ResetPlayer()
        {
            Entity[] players = Scene.GetAllEntitiesWithScript<PlayerInputHandler>();
            returnPosition = Scene.GetAllEntitiesWithScript<ReturnPosition>();

            if (players.Length > 0)
            {
                players[0].GetScript<Player>().IsTargetable = true;
                players[0].position = returnPosition[0].position;
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
            }

            GameManager.Instance.ResetPhoneBoothEvent -= ResetPlayer;
        }

        public void Activate()
        {
            myIsActivated = true;
            entity.FindChild("Audio").GetScript<AudioPhoneBooth>().ActivateRinging();
        }

        public void ReactivateAfterRound()
        {
            myIsOnCooldown = false;
            GameManager.Instance.NewRoundEvent -= ReactivateAfterRound;
            entity.FindChild("Audio").GetScript<AudioPhoneBooth>().ActivateRinging();
        }

        public override void ShowUI(bool toggleVisibility)
        {
            if (toggleVisibility)
            {
                if (myIsActivated)
                {
                    if (GameManager.Instance.PhoneBoothUsed())
                    {
                        UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.TeleportUsed);
                        return;
                    }
                    else
                    {
                        UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Teleport);
                        return;
                    }
                }
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.NoneCalling);
                return;
            }
            else if (!toggleVisibility)
            {
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
            }
        }
    }
}
