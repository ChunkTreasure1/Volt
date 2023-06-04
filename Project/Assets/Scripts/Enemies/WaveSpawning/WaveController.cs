using System.Collections.Generic;
using System.ServiceModel.Channels;
using Volt;
using Volt.Audio;

namespace Project
{
    public class WaveController : Script
    {
        private Wave myCurrentWave;
        private EnemyStats myCurrentEnemyStats;
        private float myTimeBtwWaves = 4f;

        private bool myWaveActive = false;
        private bool myGameStarted = false;

        private void OnCreate()
        {
            myCurrentEnemyStats = new EnemyStats();
        }

        private void InitializeNewRound()
        {
            myWaveActive = false;
            if(PointManager.Instance?.RoundsSurvived != 1)
            {
                AMP.PlayOneshotEvent(WWiseEvents.Play_NewRound.ToString());
            }

            entity.CreateTimer(4f, () => { StartRound(); });
        }

        public EnemyStats GetCurrentEnemyStats()
        {
            EnemyStats enemyStats = new EnemyStats(myCurrentEnemyStats);
            return enemyStats;
        }

        private void StartRound()
        {
            myCurrentWave = new Wave();
            myCurrentWave?.InitializeWave();

            myWaveActive = true;
        }

        private void OnUpdate(float deltaTime)
        {
            if (!myGameStarted)
            {
                if (GameManager.Instance.GetAllUnlockedRooms().Count > 0)
                {
                    GameManager.Instance.NewRoundEvent += InitializeNewRound;
                    GameManager.Instance.StartNewRound();

                    myGameStarted = true;
                }
            }

            if (myWaveActive)
            {
                myCurrentWave?.Update();
            }
        }
    }
}
