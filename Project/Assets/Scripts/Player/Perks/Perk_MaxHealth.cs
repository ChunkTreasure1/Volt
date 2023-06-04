using Volt;

namespace Project
{
    public class Perk_MaxHealth : Perk_Base
    {
        public override void AddModifier(PlayerStats stats)
        {
            stats.Modifiers.MaxHealthModifier = 2.0f;
            stats.CurrentHealth = stats.ModifiedMaxHealth;
        }
    }
}
