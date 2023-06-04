using System.Collections.Generic;
using System.Linq;
using Volt;

namespace Project
{
    public abstract class Interactable_Cost : Interactable_Base
    {
        public uint cost = 0;
        protected bool isVisualizing = false;

        public override void InteractEvent()
        {
            if(isVisualizing) { return; }

            if(CheckCost(cost, PointManager.Instance.currentPoints))
            {
                SpendResources();
                SpendEvent();
            }
        }
        protected bool CheckCost(uint cost, uint playerResources)
        {
            if (cost <= playerResources)
            {
                return true;
            }
            else return false;
        }
        private void SpendResources()
        {
            //Spend corresponding resources
            Log.Info("Spend corresponding resources");

            PointManager.Instance.RemovePoints(cost);
        }

        public abstract void SpendEvent();

    }
}
