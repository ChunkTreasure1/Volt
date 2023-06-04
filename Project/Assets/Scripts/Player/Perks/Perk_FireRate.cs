using Volt;

namespace Project
{
    public class Perk_FireRate: Perk_Base
    {
        public override void AddModifier(PlayerStats stats)
        {
            stats.Modifiers.FireRateModifier = 2;
        }
    }
}
