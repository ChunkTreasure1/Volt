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

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);

            return InternalCalls.Entity_HasComponent(Id, componentType.Name);
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if(!HasComponent<T>())
            {
                return null;
            }

            return new T() { entity = this }; // #TODO_Ivar: Implement caching
        }
    }
}
