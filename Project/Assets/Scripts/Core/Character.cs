using System;
using Volt;

namespace Project
{
    public enum eCharacterType
    {
        Player,
        Enemy
    }

    public abstract class Character : Script
    {
        public eCharacterType myCharacterType { get; set; }

        //public abstract void TakeDamage(float aDamage);
    }
}
