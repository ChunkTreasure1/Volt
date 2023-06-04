using System;
using System.Collections.Generic;
using System.Linq;
using System.Web.UI;
using Volt;

namespace Project
{
    public class Room : Script
    {
        public bool IsUnlocked = false;
        public Prefab EnemyPrefab;

        private List<Entity> SpawnPoints = new List<Entity>();

        private void OnCreate()
        {
            foreach (Entity child in entity.children)
            {
                if (!child.HasScript<SpawnArea>()) { continue; }

                SpawnPoints.Add(child);
            }
        }

        public Vector3 GetRandomSpawn()
        {
            if(SpawnPoints != null)
            {
                int randomSpawnPoint = Volt.Random.Range(0, SpawnPoints.Count);

                return SpawnPoints[randomSpawnPoint].position;
            }
            return Vector3.Zero;
        }

        public Prefab GetEnemyPrefab()
        {
            return EnemyPrefab != null ? EnemyPrefab : null;
        }

        public List<Entity>GetSpawnPoints()
        {
            if (SpawnPoints != null)
            {
                return SpawnPoints;
            }
            return null;
        }

        public void SpawnEnemyAtIndex(int index)
        {
            if (!IsUnlocked)
            {
                return;
            }

            if (SpawnPoints.Count > 0)
            {
                //IF NET WORK DO THIS!
                NetScene.InstantiatePrefab(EnemyPrefab.handle, SpawnPoints[index].Id);
                //ELSE DO THIS!
                //Entity enemy = Entity.Create(EnemyPrefab);
                //enemy.position = SpawnPoints[spawnPoint].position;
            }
        }

        public void SpawnEnemy()
        {
            if (!IsUnlocked) 
            {
                return;
            }

            if (SpawnPoints.Count > 0)
            {
                int spawnPoint = 0;
                if (SpawnPoints.Count != 1)
                {
                    spawnPoint = Volt.Random.Range(0, SpawnPoints.Count);
                }

                //IF NET WORK DO THIS!
                NetScene.InstantiatePrefab(EnemyPrefab.handle, SpawnPoints[spawnPoint].Id);

                //ELSE DO THIS!
                //Entity enemy = Entity.Create(EnemyPrefab);
                //enemy.position = SpawnPoints[spawnPoint].position;
            }
        }
    }
}
