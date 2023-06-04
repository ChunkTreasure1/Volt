using Volt;

namespace Project
{
    public class Perk_Stamina : Perk_Base
    {
        public override void AddModifier(PlayerStats stats)
        {
            stats.Modifiers.StaminaModifier = 1.07f;
            stats.Modifiers.StaminaRegenModifier = 2;
        }
    }
}
