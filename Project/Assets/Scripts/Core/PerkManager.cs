using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using Volt;

namespace Project
{
    public enum Perk : uint
    {
        None,
        FireRate,
        MaxHealth,
        Reload,
        Revive,
        Stamina
    };

    public class PerkManager : Script
    {
        #region Static Instance
        static PerkManager _instance;
        public static PerkManager Instance
        {
            get
            {
                if (_instance == null)
                {
                    Log.Warning("No Instance available.");
                }
                return _instance;
            }
        }
        #endregion

        private List<Perk> myPerkList = new List<Perk>();
        private List<Perk> myPlayerPerks = new List<Perk>();
        public List<Perk> PlayerPerks
        {
            get { return myPlayerPerks; }
        }

        private void OnAwake()
        {
            _instance = this;
        }

        private void OnCreate()
        {
            FillPerkList();
        }

        void OnUpdate(float aDeltaTime)
        {
            if (Input.IsKeyPressed(KeyCode.KP_1))
            {
                Entity[] players = Scene.GetAllEntitiesWithScript<PlayerInputHandler>();
                players[0].GetScript<Player>().AddPerk(GetNewPerk(Perk.FireRate));
                
            }
            if (Input.IsKeyPressed(KeyCode.KP_2))
            {
                Entity[] players = Scene.GetAllEntitiesWithScript<PlayerInputHandler>();
                players[0].GetScript<Player>().AddPerk(GetNewPerk(Perk.MaxHealth));
            }
            if (Input.IsKeyPressed(KeyCode.KP_3))
            {
                Entity[] players = Scene.GetAllEntitiesWithScript<PlayerInputHandler>();
                players[0].GetScript<Player>().AddPerk(GetNewPerk(Perk.Reload));
            }
            if (Input.IsKeyPressed(KeyCode.KP_4))
            {
                Entity[] players = Scene.GetAllEntitiesWithScript<PlayerInputHandler>();
                players[0].GetScript<Player>().AddPerk(GetNewPerk(Perk.Revive));
            }
            if (Input.IsKeyPressed(KeyCode.KP_5))
            {
                Entity[] players = Scene.GetAllEntitiesWithScript<PlayerInputHandler>();
                players[0].GetScript<Player>().AddPerk(GetNewPerk(Perk.Stamina));
            }
            if (Input.IsKeyPressed(KeyCode.KP_0))
            {
                Entity[] players = Scene.GetAllEntitiesWithScript<PlayerInputHandler>();
                players[0].GetScript<Player>().ClearPerks();
            }
        }

        public void ClearPerks()
        {
            myPlayerPerks.Clear();

            UIManager.Instance.OnPerkInteraction();
        }

        private void FillPerkList()
        {
            List<Perk> tempPerkList = new List<Perk>() { Perk.FireRate, Perk.MaxHealth, Perk.Reload, Perk.Stamina };

            int n = tempPerkList.Count;
            while (n > 1)
            {
                n--;
                int k = Volt.Random.Range(0, n + 1);
                Perk value = tempPerkList[k];
                tempPerkList[k] = tempPerkList[n];
                tempPerkList[n] = value;
            }

            myPerkList = tempPerkList;

            if(GameManager.Instance.PlayerCount == 1)
            {
                myPerkList.Add(Perk.Revive);
            }
            else
            {
                int k = Volt.Random.Range(0, myPerkList.Count);
                myPerkList.Insert(k, Perk.Revive);
            }
        }

        public Perk GetNewPerkID()
        {
            if(myPerkList.Count == 0)
            {
                FillPerkList();
            }

            Perk newPerkID = myPerkList[myPerkList.Count - 1];
            myPerkList.RemoveAt(myPerkList.Count - 1);

            return newPerkID;
        }

        public Perk_Base GetNewPerk(Perk aPerkID)
        {
            myPlayerPerks.Add(aPerkID);
            UIManager.Instance.OnPerkInteraction();
            switch (aPerkID)
            {
                case Perk.FireRate:
                    return new Perk_FireRate();
                case Perk.MaxHealth:
                    return new Perk_MaxHealth();
                case Perk.Reload:
                    return new Perk_Reload();
                case Perk.Revive:
                    return new Perk_Revive();
                case Perk.Stamina:
                    return new Perk_Stamina();
                default:
                    return null;
            }
        }
    }
}
