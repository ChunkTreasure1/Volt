using Volt;
using Volt.Audio;

namespace Project
{
    public class Interactable_PaP : Interactable_Cost
    {
        public override void SpendEvent()
        {
            PackAPunch();
        }
        private void PackAPunch()
        {
            Entity[] player = Scene.GetAllEntitiesWithScript<PlayerInputHandler>();

            if (player.Length > 0)
            {
                player[0].GetScript<Player>().GetCurrentWeapon().IsPaP = true;
                UIManager.Instance.OnWeaponInteraction();

                //AUDIO
                entity.FindChild("Audio").GetComponent<AudioSourceComponent>().PlayEvent(WWiseEvents.Play_UpgradeWeapon.ToString());

            }

            Log.Info("Weapon Upgraded");
        }

        public override void ShowUI(bool toggleVisibility)
        {
            if (toggleVisibility)
            {
                //Show appropriate UI elements
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.PackAPunch);
            }
            else if (!toggleVisibility)
            {
                //Get rid of UI element
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
            }
        }

    }
}
