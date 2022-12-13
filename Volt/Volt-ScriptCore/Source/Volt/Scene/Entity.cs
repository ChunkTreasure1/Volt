using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Volt
{
    public class Entity
    {
        public readonly uint Id;
        public static uint Null = 0;

        private Dictionary<string, Component> myComponentCache;


        protected Entity() { Id = 0; }

        internal Entity(uint id)
        {
            Id = id;
        }

        public Vector3 position
        {
            get
            {
                InternalCalls.TransformComponent_GetPosition(Id, out Vector3 pos);
                return pos;
            }

            set
            {
                InternalCalls.TransformComponent_SetPosition(Id, ref value);
            }
        }

        public Quaternion rotation
        {
            get
            {
            }

            set
            {

            }
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);

            return InternalCalls.Entity_HasComponent(Id, componentType.Name);
        }

        public void RemoveComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            if (!HasComponent<T>())
            {
                Log.Error($"Component with name {componentType.Name} does not exist on entity {Id}");
                return;
            }

            InternalCalls.Entity_RemoveComponent(Id, componentType.Name);
        }

        public T AddComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            if (HasComponent<T>())
            {
                return myComponentCache[componentType.Name] as T;
            }

            InternalCalls.Entity_AddComponent(Id, componentType.Name);

            T newComp = new T() { entity = this };
            myComponentCache.Add(componentType.Name, newComp);

            return newComp;
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
            {
                return null;
            }

            Type componentType = typeof(T);
            if (myComponentCache.ContainsKey(componentType.Name))
            {
                return myComponentCache[componentType.Name] as T;
            }

            T newComp = new T() { entity = this };
            myComponentCache.Add(componentType.Name, newComp);
            return newComp;
        }
    }
}
