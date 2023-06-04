using Volt;

namespace Project
{
    public class Perk_Reload : Perk_Base
    {
        public override void AddModifier(PlayerStats stats)
        {
            stats.Modifiers.ReloadModifier = 0.5f;
        }
    }
}
