using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Volt
{
    [EngineScript]
    public class Scene : Asset
    {
        public Scene() { }
        public Scene(AssetHandle handle) : base(handle) { }

        static private List<EntityTimer> myTimers = new List<EntityTimer>();
        static private Dictionary<uint, List<EntityTimer>> myEntityTimers = new Dictionary<uint, List<EntityTimer>>();

        static public void Load(Scene aScene)
        {
            aScene.Load();
        }

        public static void Save(Scene aScene)
        {
            aScene.Save();
        }

        public static Entity InstantiateSplitMesh(AssetHandle meshHandle)
        {
            Entity shattredEnt = InternalCalls.Scene_InstantiateSplitMesh(meshHandle);
            return shattredEnt;
        }

        public static void CreateDynamicPhysicsActor(Entity entity)
        {
            InternalCalls.Scene_CreateDynamicPhysicsActor(entity.Id);
        }

        static public Entity[] GetAllEntities()
        {
            return InternalCalls.Scene_GetAllEntities();
        }

        static public Entity[] GetAllEntitiesWithComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return InternalCalls.Scene_GetAllEntitiesWithComponent(componentType.Name);
        }

        static public Entity[] GetAllEntitiesWithScript<T>() where T : Script, new()
        {
            Type scriptType = typeof(T);
            return InternalCalls.Scene_GetAllEntitiesWithScript(scriptType.Namespace + "." + scriptType.Name);
        }

        public void Load()
        {
            if (handle != 0)
            {
                InternalCalls.Scene_Load(handle);
            }
        }

        public void Save()
        {
            if (handle != 0)
            {
                InternalCalls.Scene_Save(handle);
            }
        }

        static internal void UpdateSceneTimers()
        {
            for (int i = myTimers.Count - 1; i >= 0; i--)
            {
                myTimers[i].time -= Time.deltaTime;
                if (myTimers[i].time <= 0f)
                {
                    myTimers[i].onTimerEndFunction();
                    myTimers.RemoveAt(i);
                }
            }

            foreach (KeyValuePair<uint, List<EntityTimer>> entry in myEntityTimers)
            {
                for(int i = entry.Value.Count - 1; i >= 0; i--)
                {
                    entry.Value[i].time -= Time.deltaTime;
                    if (entry.Value[i].time <= 0f)
                    {
                        entry.Value[i].onTimerEndFunction();
                        myEntityTimers[entry.Key].RemoveAt(i);
                    }
                }
            }
        }

        static public EntityTimer CreateTimer(float time, Action functionToCall)
        {
            EntityTimer timer = new EntityTimer(time, functionToCall);
            myTimers.Add(timer);
            return timer;
        }

        internal static void AddTimer(uint entityId, EntityTimer timer)
        {
            if (!myEntityTimers.ContainsKey(entityId))
            {
                myEntityTimers.Add(entityId, new List<EntityTimer>());
            }

            myEntityTimers[entityId].Add(timer);
        }

        internal static void RemoveTimer(uint entityId, EntityTimer timer)
        {
            myEntityTimers[entityId].Remove(timer);
        }
    }
}