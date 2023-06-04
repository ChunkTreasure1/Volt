using System;
using System.Collections.Generic;
using System.ComponentModel;
using Volt;

namespace Project
{
    public class Stats
    {
        public float MaxHealth = 100;
        public float CurrentHealth;
    }

    public class Modifier
    {
        public float MovementModifier = 1;
        public float MaxHealthModifier = 1;
        public float FireRateModifier = 1;
        public float ReloadModifier = 1;
        public float ReviveModifier = 1;
        public float StaminaModifier = 1;
        public float StaminaRegenModifier = 1;
    } 

    public class PlayerStats : Stats
    {
        public enum PlayerType : int
        {
            Beef,
            Speed
        }

        public PlayerType myPlayerType;
        public Modifier Modifiers;

        private float myWalkMovementSpeed = 500f;
        public float WalkMovementSpeed
        {
            get { return myWalkMovementSpeed * Modifiers.MovementModifier; }
            protected set { myWalkMovementSpeed = value; }
        }

        private float myRunningMovementSpeed = 750f;
        public float RunningMovementSpeed
        {
            get { return myRunningMovementSpeed * Modifiers.MovementModifier;}
            protected set { myRunningMovementSpeed = value;}
        }

        public float ModifiedMaxHealth
        {
            get { return MaxHealth * Modifiers.MaxHealthModifier; }
        }

        private float myAimingMovementSpeed = 350f;
        public float AimingMovementSpeed
        {
            get { return myAimingMovementSpeed * Modifiers.MovementModifier; }
            protected set { myAimingMovementSpeed = value; }
        }

        private float myMovementAcceleration = 5000f;
        public float MovementAcceleration
        {
            get { return myMovementAcceleration; }
            protected set { myMovementAcceleration = value; }
        }

        private float myInAirAcceleration = 1000f;
        public float InAirAcceleration
        {
            get { return myInAirAcceleration; }
            protected set { myInAirAcceleration = value; }
        }

        private float myStamina = 4f;
        public float Stamina
        {
            get { return myStamina * Modifiers.StaminaModifier; }
            protected set { myStamina = value; }
        }

        private float myStaminaRegenerationRate = 1f;
        public float StaminaRegenerationRate
        {
            get { return myStaminaRegenerationRate * Modifiers.StaminaRegenModifier; }
            protected set { myStaminaRegenerationRate = value; }
        }

        private float myRunCooldown = 1f;
        public float RunCooldown
        {
            get { return myRunCooldown; }
            protected set { myRunCooldown = value; }
        }

        private float myJumpHeight  = 60f;
        public float JumpHeight
        {
            get { return myJumpHeight; }
            protected set { myJumpHeight = value; }
        }

        private float myGravityMultiplier = 1f;
        public float GravityMultiplier
        {
            get { return myGravityMultiplier; }
            protected set { myGravityMultiplier = value; }
        }

        private float myMeleeDamage = 150f;
        public float MeleeDamage
        {
            get { return myMeleeDamage; }
            protected set { myMeleeDamage = value; }
        }

        private float myHealthRegen = 25f;
        public float HealthRegen
        {
            get { return myHealthRegen; }
            protected set { myHealthRegen = value; }
        }

        private float myReviveTime = 3f;
        public float ReviveTime
        {
            get { return myReviveTime; }
            protected set { myReviveTime = value; }
        }

        private float myBleedoutTime = 45f;
        public float BleedoutTime
        {
            get { return myBleedoutTime; }
            protected set { myBleedoutTime = value; }
        }

        public void Initialize(PlayerType aType)
        {
            Modifiers = new Modifier();

            if (aType == PlayerType.Beef)
            {
                // Health
                MaxHealth += 50;

                // Melee damage
                MeleeDamage += MeleeDamage * 2;

                myPlayerType = PlayerType.Beef;
            }

            if (aType == PlayerType.Speed)
            {
                // Movement
                WalkMovementSpeed += WalkMovementSpeed * 0.03f;
                MovementAcceleration += MovementAcceleration * 0.03f;
                RunningMovementSpeed += RunningMovementSpeed * 0.03f;
                Stamina += Stamina + 0.5f;

                myPlayerType = PlayerType.Speed;
            }

            CurrentHealth = MaxHealth;
        }
    }

    public class EnemyStats : Stats
    {
        public enum EnemyType
        {
            NormalEnemy
        }

        public float WalkMovementSpeed = 300;
        public float RunMovementSpeed = 450;
        public float AttackDistance = 200f;
        public float TimeBtwAttacks = 1.2f;

        public EnemyStats() 
        {
            CurrentHealth = MaxHealth;    
        }

        public EnemyStats(EnemyStats aStat)
        {
            WalkMovementSpeed = aStat.WalkMovementSpeed;
            RunMovementSpeed = aStat.RunMovementSpeed;
            MaxHealth = aStat.MaxHealth;
            CurrentHealth = aStat.CurrentHealth;
        }
    }
}
