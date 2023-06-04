using System.Collections.Generic;
using System.Data.Metadata.Edm;
using System.Linq;
using Volt;

namespace Project
{
    public class Interactable_Perk : Interactable_Cost
    {
        private enum PerkMachineState
        {
            None,
            Usable,
            Randomizing,
            PickUpable,
        }

        PerkMachineState myCurrentState = PerkMachineState.Usable;
        Perk myLoadedPerk = Perk.None;

        public override void SpendEvent()
        {
            switch (myCurrentState)
            {
                case PerkMachineState.Usable:
                    RandomizePerk();
                    break;
                case PerkMachineState.Randomizing:
                    //RANDOMIZE WAITING
                    break;
                case PerkMachineState.PickUpable:
                    GetPerk();
                    break;
            }
            UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
        }

        private void RandomizePerk()
        {
            myCurrentState = PerkMachineState.PickUpable;
            cost = 0;
            myLoadedPerk = PerkManager.Instance.GetNewPerkID();
        }

        private void GetPerk()
        {
            Entity[] players = Scene.GetAllEntitiesWithScript<PlayerInputHandler>();
            players[0].GetScript<Player>().AddPerk(PerkManager.Instance.GetNewPerk(myLoadedPerk));
            cost = 1500;

            myCurrentState = PerkMachineState.Usable;

            //Log.Info("Give player the randomised perk");
        }

        public override void ShowUI(bool toggleVisibility)
        {
            if (toggleVisibility)
            {
                //Show appropriate UI elements
                switch (myCurrentState)
                {
                    case PerkMachineState.Usable:
                        UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.BuyPerk);
                        break;
                    case PerkMachineState.Randomizing:
                        break;
                    case PerkMachineState.PickUpable:

                        switch (myLoadedPerk)
                        {
                            case Perk.FireRate:
                                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.PerkFireRate);
                                break;
                            case Perk.MaxHealth:
                                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.PerkMaxHealth);
                                break;
                            case Perk.Reload:
                                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.PerkReload);
                                break;
                            case Perk.Revive:
                                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.PerkRevive);
                                break;
                            case Perk.Stamina:
                                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.PerkStamina);
                                break;
                        }
                        break;
                }

            }
            else if (!toggleVisibility)
            {
                //Get rid of UI element
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
            }
        }

    }
}
