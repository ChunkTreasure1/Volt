using System;
using System.Collections.Generic;
using System.Data.Linq;
using System.Linq;
using System.Security.Policy;
using System.Windows.Forms;
using Volt;

namespace Project
{
    public enum PowerUps : uint
    {
        None, InstaKill, DoublePoints, MaxAmmo, Nuke, Carpenter
    };

    public class GameManager : Script
    {
        #region Static Instance
        static GameManager _instance;
        public static GameManager Instance
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

        private List<Room> Rooms = new List<Room>();

        private void OnAwake()
        {
            _instance = this;
        }

        public List<Entity> Players;
        private Entity mySpectateCamera;

        private WaveController myWaveController;

        public int PlayerCount = 1;

        [RepUpdate] public uint myCurrentRound = 0;

        public float LastRoundEnemyHealth = 150;
        public float CurrentRoundHealth = 150;

        private void OnCreate()
        {
            Players = Scene.GetAllEntitiesWithScript<Player>().ToList();
            PlayerCount = Players.Count;

            if(PlayerCount >= 2)
            {
                Entity[] specCam = Scene.GetAllEntitiesWithScript<SpectateCamera>();
                if(specCam != null && specCam.Length > 0)
                {
                    mySpectateCamera = specCam[0];
                }
            }

            Entity[] rooms = Scene.GetAllEntitiesWithScript<Room>();
            
            foreach(Entity room in rooms)
            {
                if(room != null && room.HasScript<Room>())
                {
                    Rooms.Add(room.GetScript<Room>());
                }
            }

            myWaveController = entity?.GetScript<WaveController>();
        }

        void OnUpdate(float aDeltaTime)
        {
            UpdatePowerUps(aDeltaTime);
        }

        private float CalculateMaxHealth(uint currentRound)
        {
            float resultHealth = 150;

            if (currentRound < 10)
            {
                resultHealth += 100 * (currentRound - 1);
            }
            else
            {
                resultHealth = GameManager.Instance.LastRoundEnemyHealth * 1.1f;
            }

            return resultHealth;
        }

        public void StartGame()
        {
            StartGameEvent?.Invoke();
        }

        public WaveController GetWaveController()
        {
            return myWaveController != null ? myWaveController : null;
        }

        public uint GetCurrentRound()
        {
            return myCurrentRound;
        }

        public List<Room> GetAllRooms()
        {
            return Rooms != null ? Rooms : new List<Room>();
        }

        public List<Room> GetAllUnlockedRooms()
        {
            List<Room> unlockedRooms = new List<Room>();

            foreach(Room room in Rooms)
            {
                if (room.IsUnlocked)
                {
                    unlockedRooms.Add(room);
                }
            }

            return unlockedRooms.Count <= 0 ? new List<Room>() : unlockedRooms;
        }

        //POWER-UPS
        [RepNotify]
        public bool InstaKill = false;
        private float myInstaKillTimer = 30;

        [RepNotify]
        public bool DoublePoints = false;
        private float myDoublePointsTimer = 30;

        [RepNotify]
        public bool MaxAmmo = false;
        [RepNotify]
        public bool Nuke = false;
        [RepNotify]
        public bool Carpenter = false;

        private uint myAmountofPowerups = 0;

        public delegate void InstaKillHandler(object sender, EventArgs e, bool isStarted);
        public delegate void DoublePointHandler(object sender, EventArgs e, bool isStarted);
        public delegate void MaxAmmoHandler();
        public delegate void NukeHandler();
        public delegate void CarpenterHandler();
        public delegate void PhoneBoothHandler();
        public delegate void ResetPhoneBoothHandler();

        public delegate void EnemyDeathHandler();
        public delegate void NewRoundHandler();

        public delegate void StartGameHandler();

        public ResetPhoneBoothHandler ResetPhoneBoothEvent;
        public PhoneBoothHandler PhoneBoothEvent;
        public InstaKillHandler InstaKillEvent;
        public DoublePointHandler DoublePointEvent;
        public MaxAmmoHandler MaxAmmoEvent;
        public NukeHandler NukeEvent;
        public CarpenterHandler CarpenterEvent;
        public EnemyDeathHandler EnemyDeathEvent;
        public NewRoundHandler NewRoundEvent;
        public StartGameHandler StartGameEvent;

        private bool myTeleportEventActive = false;

        void UpdatePowerUps(float aDeltaTime)
        {
            if (InstaKill)
            {
                myInstaKillTimer -= aDeltaTime;

                if(myInstaKillTimer <= 0)
                {
                    CallInstaKillEvent(false);
                }
            }

            if (DoublePoints)
            {
                myDoublePointsTimer -= aDeltaTime;

                if(myDoublePointsTimer <= 0)
                {
                    CallDoublePointEvent(false);
                }
            }
        }

        public void ResetPowerUpsRound()
        {
            myAmountofPowerups = 0;
        }

        public bool PhoneBoothUsed()
        {
            return myTeleportEventActive;
        }

        public void SpawnDrops(Entity aDeadEnt)
        {
            if(myAmountofPowerups == 4) { return; }
            if(Volt.Random.Range(0, 100) > 2) { return; }

            uint powerUpToSpawn = (uint)Volt.Random.Range(1, 6);

            //SPAWN A PICK UP
            Entity PUEnt;
            switch ((PowerUps)powerUpToSpawn)
            {
                case PowerUps.InstaKill:
                    Prefab IKPrefab = AssetManager.GetAsset<Prefab>("Assets/Prefabs/PowerUps/PowerUp_InstaKill.vtprefab");
                    NetScene.InstantiatePrefab(IKPrefab.handle, aDeadEnt.Id);
                    break;
                case PowerUps.DoublePoints:
                    Prefab DPPrefab = AssetManager.GetAsset<Prefab>("Assets/Prefabs/PowerUps/PowerUp_DoublePoints.vtprefab");
                    NetScene.InstantiatePrefab(DPPrefab.handle, aDeadEnt.Id);
                    break;
                case PowerUps.MaxAmmo:
                    Prefab MAPrefab = AssetManager.GetAsset<Prefab>("Assets/Prefabs/PowerUps/PowerUp_MaxAmmo.vtprefab");
                    NetScene.InstantiatePrefab(MAPrefab.handle, aDeadEnt.Id);
                    break;
                case PowerUps.Nuke:
                    Prefab NUPrefab = AssetManager.GetAsset<Prefab>("Assets/Prefabs/PowerUps/PowerUp_Nuke.vtprefab");
                    NetScene.InstantiatePrefab(NUPrefab.handle, aDeadEnt.Id);
                    break;
                case PowerUps.Carpenter:
                    Prefab CAPrefab = AssetManager.GetAsset<Prefab>("Assets/Prefabs/PowerUps/PowerUp_Carpenter.vtprefab");
                    NetScene.InstantiatePrefab(CAPrefab.handle, aDeadEnt.Id);
                    break;
                default:
                    return;
            }

            myAmountofPowerups++;
            return;
        }

        public void CallEnemyDeathEvent()
        {
            EnemyDeathEvent?.Invoke();
        }
        public void StartNewRound()
        {
            CleanScene();
            myCurrentRound++;

            LastRoundEnemyHealth = CurrentRoundHealth;

            CurrentRoundHealth = CalculateMaxHealth(myCurrentRound);

            ResetPowerUpsRound();
            NewRoundEvent?.Invoke();
            myTeleportEventActive = false;
        }

        private void CleanScene()
        {
            Entity[] enemies = Scene.GetAllEntitiesWithScript<EnemyBase>();
            foreach(Entity enemy in enemies)
            {
                NetScene.DestroyPrefabFromLocal(enemy.Id);
            }
        }

        public bool CallPhoneBoothEvent()
        {
            if (myTeleportEventActive) { return false; }
            
            PhoneBoothEvent?.Invoke();

            entity.CreateTimer(Interactable_PhoneBooth.TimeUntilReturn, () => { ResetPhoneBoothEvent?.Invoke(); });
            myTeleportEventActive = true;

            return true;
        }

        public void CallInstaKillEvent(bool aState)
        {
            InstaKill = aState;
            myInstaKillTimer = 30;
            InstaKillEvent?.Invoke(this, EventArgs.Empty, aState);
            UIManager.Instance.PowerUpInteractionEvent();
        }
        public void CallDoublePointEvent(bool aState)
        {
            DoublePoints = aState;
            myDoublePointsTimer = 30;
            DoublePointEvent?.Invoke(this, EventArgs.Empty, aState);
            UIManager.Instance.PowerUpInteractionEvent();
        }
        public void CallMaxAmmoEvent()
        {
            MaxAmmoEvent?.Invoke();
        }
        public void CallNukeEvent()
        {
            NukeEvent?.Invoke();
            PointManager.Instance?.AddPoints(200);

        }
        public void CallCarpenterEvent()
        {
            CarpenterEvent?.Invoke();
            PointManager.Instance?.AddPoints(200);
        }

        private bool myPlayerInPauseMenu = true;
        public bool playerInPauseMenu
        {
            get { return playerInPauseMenu; }
        }
        public void SetPauseMenuState(bool aState)
        {
            myPlayerInPauseMenu = aState;
        }


        //END OF GAME
        public delegate void EndGameHandler();
        public EndGameHandler EndGameEvent;

        public void EndGame()
        {
            if(Players.Count >= 2)
            {
                mySpectateCamera.GetScript<SpectateCamera>().Active = true;
            }

            EndGameEvent?.Invoke();
        }
    }
}
