using System.Collections.Generic;
using System.Linq;
using Volt;

namespace Project
{
    public class Interactable_Weapon : Interactable_Cost
    {
        public WeaponId WeaponBuy;
        
        public override void SpendEvent()
        {
            GetWeapon();
            UIManager.Instance.WeaponInteractionEvent();
            UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
        }

        public override void ShowUI(bool toggleVisibility)
        {
            if (toggleVisibility)
            {
                //Show appropriate UI elements

                switch (WeaponBuy)
                {
                    case WeaponId.Galil:
                        UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.BuyAssault);
                        break;
                    case WeaponId.Kar98k:
                        UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.BuyBoltAction);
                        break;
                    case WeaponId.M1911:
                        UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.BuyPistol);
                        break;
                    case WeaponId.Spas12:
                        UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.BuyShotgun);
                        break;
                }
            }
            else if (!toggleVisibility)
            {
                //Get rid of UI element
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
            }
        }

        private void GetWeapon()
        {
            Entity[] player = Scene.GetAllEntitiesWithScript<PlayerInputHandler>();

            if(player.Length > 0)
            {
                WeaponBehaviour newWeapon = WeaponManager.Instance.CreateWeapon(WeaponBuy);
                if (player[0].HasScript<Player>())
                {
                    player[0].GetScript<Player>().SetCurrentWeapon(newWeapon);
                }
            }

            Log.Info("Give player the weapon");
        }
    }
}
