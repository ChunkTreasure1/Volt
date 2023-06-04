using System.Web.Http.ValueProviders;
using System.Windows.Forms;
using Volt;

namespace Project
{
    public class Interactable_MysterBox : Interactable_Cost
    {
        public enum eBoxState
        {
            Closed,
            Randomizing,
            Grabable
        }

        WeaponBehaviour myRandomWeapon = null;
        private Entity myWeaponVisualizer;
        private eBoxState myBoxState = eBoxState.Closed;

        public override void SpendEvent()
        {
            if (myBoxState == eBoxState.Closed)
            {
                Use();
            }
            else if (myBoxState == eBoxState.Grabable)
            {
                GrabWeapon();
            }
            UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
        }

        private void Use()
        {
            WeaponId id = (WeaponId)Volt.Random.Range(1, (int)WeaponId.COUNT);

            myRandomWeapon = WeaponManager.Instance.CreateWeapon(id);

            entity.CreateTimer(4f, () => { SetState(eBoxState.Grabable); });
            entity.CreateTimer(16f, () => { SetState(eBoxState.Closed); });

            UIManager.Instance.OnWeaponInteraction();

            SetState(eBoxState.Randomizing);

            isVisualizing = true;
        }

        private void SetState(eBoxState aState)
        {
            myBoxState = aState;
        }

        protected override void Update()
        {
            switch (myBoxState)
            {
                case eBoxState.Closed:
                    isVisualizing = false;
                    break;
                case eBoxState.Randomizing:
                    RandomizeWeapon();
                    break;
                case eBoxState.Grabable:
                    ShowWeaponToGrab();
                    break;
                default:
                    Log.Error("Eyo wtf box shiet");
                    break;
            }
        }

        private void RandomizeWeapon()
        {
            isVisualizing = true;
            // Visual showcase like in cod maybe ?
        }

        private void ShowWeaponToGrab()
        {
            cost = 0;
            isVisualizing = false;
            // Visual showcase like in cod maybe ?
        }

        private void GrabWeapon()
        {
            Entity[] player = Scene.GetAllEntitiesWithScript<PlayerInputHandler>();
            if (player.Length > 0)
            {
                if (myRandomWeapon != null)
                {
                    player[0].GetScript<Player>().SetCurrentWeapon(myRandomWeapon);
                }
            }
            cost = 950;
            UIManager.Instance.OnWeaponInteraction();
        }

        public override void ShowUI(bool toggleVisibility)
        {
            if (toggleVisibility)
            {
                if (myBoxState == eBoxState.Closed)
                {
                    UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.BuyMysteryWeapon);
                    return;
                }

                if (myBoxState == eBoxState.Randomizing)
                {
                    return;
                }

                if (myBoxState == eBoxState.Grabable)
                {
                    switch (myRandomWeapon.Id)
                    {
                        case WeaponId.Galil:
                            UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.PickUpAssult);
                            break;
                        case WeaponId.Spas12:
                            UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.PickUpBoltAction);
                            break;
                        case WeaponId.M1911:
                            UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.PickUpPistol);
                            break;
                        case WeaponId.Thunder:
                            UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.PickUpThunderGun);
                            break;
                        case WeaponId.Kar98k:
                            UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.PickUpShotgun);
                            break;
                    }
                    return;
                }

            }
            else
            {
                UIManager.Instance.OnPlayerInteraction(UIManager.InteractionTypes.Disable);
            }
        }

    }
}
