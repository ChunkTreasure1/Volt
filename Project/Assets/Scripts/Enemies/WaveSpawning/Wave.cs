using System;
using System.Collections.Generic;
using System.Linq;
using Volt;

namespace Project
{
    public delegate void StartNewRoundDelegate();

    public class Wave
    {
        List<uint> myDeadEnemies = new List<uint>();

        //Wave Stats
        private uint myEnemiesAlive = 0;
        private uint myMaxEnemiesAlive = 0;
        private uint myEnemiesToKill = 0;

        private float myTimeBtwSpawns = 1f;

        //Private State Stuff
        private GameManager myGameManager;
        private WaveController myWaveController;

        #region Public Functions

        public void InitializeWave()
        {
            myGameManager = GameManager.Instance;
            myWaveController = myGameManager.GetWaveController();

            uint currentRound = myGameManager.GetCurrentRound();

            //Set New Round Stats
            myMaxEnemiesAlive = (24 + 6 * ((uint)myGameManager.PlayerCount - 1));

            myEnemiesToKill = CalculateEnemiesToKill(currentRound);
            myMaxEnemiesAlive = CalculateMaxEnemiesAlive(currentRound);

            myGameManager.EnemyDeathEvent += OnEnemyDeath;

            Log.Info("Round: " + myGameManager.GetCurrentRound());
            Log.Info("Enemies To Kill: [" + myEnemiesToKill.ToString() + "]");
        }

        public void Update()
        {
            //Log.Trace("Enemies To Kill: [" + myEnemiesToKill.ToString() + "]");
            //Log.Trace("Enemies Alive: [" + myEnemiesAlive.ToString() + "]");
            //Log.Trace("My Max Enemies Alive: [" + myEnemiesToKill.ToString() + "]");

            if (myEnemiesToKill <= 0)
            {
                myGameManager.StartNewRound();
            }

            if (myTimeBtwSpawns <= 0 && myEnemiesAlive < myMaxEnemiesAlive && myEnemiesAlive < myEnemiesToKill)
            {
                SpawnEnemy();
            }
            else
            {
                myTimeBtwSpawns -= Time.deltaTime;
            }
        }

        #endregion

        #region PrivateFunctions

        private uint CalculateMaxEnemiesAlive(uint currentRound)
        {
            float resultAmount = 0f;
            if (currentRound > 4) { return myMaxEnemiesAlive; }

            switch (currentRound)
            {
                case 1:
                    resultAmount = myMaxEnemiesAlive * 0.2f;
                    break;
                case 2:
                    resultAmount = myMaxEnemiesAlive * 0.4f;
                    break;
                case 3:
                    resultAmount = myMaxEnemiesAlive * 0.6f;
                    break;
                case 4:
                    resultAmount = myMaxEnemiesAlive * 0.8f;
                    break;
            }

            resultAmount = Mathf.Floor(resultAmount);
            return (uint)resultAmount;
        }

        private uint CalculateEnemiesToKill(uint currentRound)
        {
            float resultAmount = 0f;

            if (currentRound < 5)
            {
                switch (currentRound)
                {
                    case 1:
                        resultAmount = myMaxEnemiesAlive * 0.2f;
                        break;
                    case 2:
                        resultAmount = myMaxEnemiesAlive * 0.4f;
                        break;
                    case 3:
                        resultAmount = myMaxEnemiesAlive * 0.6f;
                        break;
                    case 4:
                        resultAmount = myMaxEnemiesAlive * 0.8f;
                        break;
                }
            }
            else if (currentRound < 10)
            {
                resultAmount = myMaxEnemiesAlive;
            }
            else
            {
                float multiplier = currentRound * 0.15f;
                resultAmount = multiplier * myMaxEnemiesAlive;
            }

            resultAmount = Mathf.Floor(resultAmount);
            return (uint)resultAmount;
        }

        private void SpawnEnemy()
        {
            List<Room> rooms = myGameManager.GetAllUnlockedRooms();

            if (rooms.Count == 0) { return; }

            //int randomRoom = Volt.Random.Range(0, rooms.Count);
            Entity randomSpawnPoint = CalculateRandomSpawn(rooms);

            NetScene.InstantiatePrefab(rooms[0].GetEnemyPrefab().handle, randomSpawnPoint.Id);

            myEnemiesAlive++;
            myTimeBtwSpawns = Volt.Random.Range(1, 4);

            return;
        }

        private Entity CalculateRandomSpawn(List<Room> unlockedRooms)
        {
            List<Entity> playerEntitys = Scene.GetAllEntitiesWithScript<Player>().ToList();

            foreach (Entity playerEnt in playerEntitys)
            {
                if (!playerEnt.GetScript<Player>().IsTargetable)
                {
                    playerEntitys.Remove(playerEnt);
                }
            }

            if (playerEntitys.Count <= 0) return unlockedRooms[0].GetSpawnPoints()[0];

            Entity chosenPlayer = playerEntitys[0];

            if (playerEntitys.Count > 1)
            {
                chosenPlayer = playerEntitys[Volt.Random.Range(0, playerEntitys.Count)];
            }

            List<(Entity, float)> distancesToSpawnPoints = new List<(Entity, float)>();

            foreach (Room room in unlockedRooms)
            {
                foreach (Entity spawnPointEnt in room.GetSpawnPoints())
                {
                    float distToSpawnPoint = (chosenPlayer.position - spawnPointEnt.position).Length();

                    distancesToSpawnPoints.Add((spawnPointEnt, distToSpawnPoint));
                }
            }

            distancesToSpawnPoints.OrderBy(tuple => tuple.Item2);

            int randomSpawn = Volt.Random.Range(0, 4);

            return distancesToSpawnPoints[randomSpawn].Item1;
        }

        public void OnEnemyDeath()
        {
            myEnemiesAlive--;
            myEnemiesToKill--;
        }
        #endregion
    }
}
