using System;
using System.Collections.Generic;
using System.Linq;

namespace Volt
{
    public class EntityTimer
    {
        public float time;
        internal Action onTimerEndFunction;

        internal EntityTimer(float time, Action function)
        {
            this.time = time;
            this.onTimerEndFunction = function;
        }
    }

    public class Entity
    {
        public readonly uint Id = UInt32.MaxValue;
        ulong[] ScriptIds;

        private Dictionary<string, Component> m_componentCache = new Dictionary<string, Component>();

        public Entity() { }
        public Entity(uint id) { Id = id; }

        internal Entity(uint id, ulong[] scriptIds)
        {
            Id = id;
            ScriptIds = scriptIds;
        }

        public static bool Exists(Entity entity)
        {
            return InternalCalls.Entity_IsValid(entity.Id);
        }

        public static Entity Find(string name)
        {
            object entity = InternalCalls.Entity_FindByName(name);
            return entity as Entity;
        }

        public static Entity Find(uint entityId)
        {
            object entity = InternalCalls.Entity_FindById(entityId);
            return entity as Entity;
        }

        public static Entity FindChild(Entity parentEntity, string childName)
        {
            return parentEntity.FindChild(childName);
        }

        public static Entity Create(string name = "New Entity")
        {
            object entity = InternalCalls.Entity_CreateNewEntity(name);
            return entity as Entity;
        }

        public static Entity Create(Prefab prefab)
        {
            return prefab?.Instantiate();
        }

        public static void Destroy(Entity entity)
        {
            if (entity == null)
            {
                return;
            }

            entity.parent = null;
            InternalCalls.Entity_DeleteEntity(entity.Id);
        }

        public static Entity Clone(Entity entity)
        {
            object result = InternalCalls.Entity_Clone(entity.Id);
            return result as Entity;
        }

        public bool Exists()
        {
            return Entity.Exists(this);
        }

        public EntityTimer CreateTimer(float time, Action functionToCall)
        {
            EntityTimer timer = new EntityTimer(time, functionToCall);
            Scene.AddTimer(Id, timer);
            return timer;
        }

        public void CancelTimer(EntityTimer timer)
        {
            Scene.RemoveTimer(Id, timer);
        }


        public Entity FindChild(string name)
        {
            object entity = InternalCalls.RelationshipComponent_FindByName(Id, name);
            return entity as Entity;
        }

        public string name
        {
            get
            {
                return GetComponent<TagComponent>().tag;
            }

            set
            {
                GetComponent<TagComponent>().tag = value;
            }
        }

        public Entity parent
        {
            get
            {
                return GetComponent<RelationshipComponent>().parent;
            }

            set
            {
                GetComponent<RelationshipComponent>().parent = value;
            }
        }

        public Entity[] children
        {
            get
            {
                if (!HasComponent<RelationshipComponent>())
                {
                    AddComponent<RelationshipComponent>();
                }

                return GetComponent<RelationshipComponent>().children;
            }
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
                InternalCalls.TransformComponent_GetRotation(Id, out Quaternion rot);
                return rot;
            }

            set
            {
                InternalCalls.TransformComponent_SetRotation(Id, ref value);
            }
        }

        public Vector3 scale
        {
            get
            {
                InternalCalls.TransformComponent_GetScale(Id, out Vector3 scale);
                return scale;
            }

            set
            {
                InternalCalls.TransformComponent_SetScale(Id, ref value);
            }
        }

        public Vector3 up
        {
            get
            {
                InternalCalls.TransformComponent_GetUp(Id, out Vector3 up);
                return up;
            }

            private set
            {

            }
        }

        public Vector3 right
        {
            get
            {
                InternalCalls.TransformComponent_GetRight(Id, out Vector3 right);
                return right;
            }

            private set
            {

            }
        }

        public Vector3 forward
        {
            get
            {
                InternalCalls.TransformComponent_GetForward(Id, out Vector3 forward);
                return forward;
            }

            private set
            {

            }
        }

        public Vector3 localPosition
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalPosition(Id, out Vector3 pos);
                return pos;
            }

            set
            {
                InternalCalls.TransformComponent_SetLocalPosition(Id, ref value);
            }
        }

        public Quaternion localRotation
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalRotation(Id, out Quaternion rot);
                return rot;
            }

            set
            {
                InternalCalls.TransformComponent_SetLocalRotation(Id, ref value);
            }
        }

        public Vector3 localScale
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalScale(Id, out Vector3 scale);
                return scale;
            }

            set
            {
                InternalCalls.TransformComponent_SetLocalScale(Id, ref value);
            }
        }

        public Vector3 localUp
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalUp(Id, out Vector3 up);
                return up;
            }

            private set
            {

            }
        }

        public Vector3 localRight
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalRight(Id, out Vector3 right);
                return right;
            }

            private set
            {

            }
        }

        public Vector3 localForward
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalForward(Id, out Vector3 forward);
                return forward;
            }

            private set
            {

            }
        }

        public bool visible
        {
            get
            {
                return InternalCalls.TransformComponent_GetVisible(Id);
            }

            set
            {
                InternalCalls.TransformComponent_SetVisible(Id, value);
            }
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            T tempVar = new T();

            return InternalCalls.Entity_HasComponent(Id, tempVar.GUID);
        }

        public void RemoveComponent<T>() where T : Component, new()
        {
            T tempVar = new T();

            if (!HasComponent<T>())
            {
                Type componentType = typeof(T);
                Log.Error($"Component with name {componentType.Name} does not exist on entity {Id}");
                return;
            }

            InternalCalls.Entity_RemoveComponent(Id, tempVar.GUID);
        }

        public T AddComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            if (HasComponent<T>())
            {
                return m_componentCache[componentType.Name] as T;
            }

            if (m_componentCache == null)
            {
                this.m_componentCache = new Dictionary<string, Component>();
            }

            T newComp = new T() { entity = this };
            InternalCalls.Entity_AddComponent(Id, newComp.GUID);


            m_componentCache[componentType.Name] = newComp;

            return newComp;
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
            {
                return null;
            }

            if (m_componentCache == null)
            {
                this.m_componentCache = new Dictionary<string, Component>();
            }

            Type componentType = typeof(T);
            if (m_componentCache.ContainsKey(componentType.Name))
            {
                return m_componentCache[componentType.Name] as T;
            }

            T newComp = new T() { entity = this };
            m_componentCache.Add(componentType.Name, newComp);
            return newComp;
        }

        public T GetComponentInChildren<T>() where T : Component, new()
        {
            foreach (Entity child in children)
            {
                if (!child.HasComponent<T>())
                {
                    continue;
                }

                return child.GetComponent<T>();
            }
            return null;
        }

        public bool HasScript<T>() where T : Script, new()
        {
            Type componentType = typeof(T);
            return InternalCalls.Entity_HasScript(Id, componentType.Namespace + "." + componentType.Name);
        }

        // Works but causes timers to stop working
        //public void RemoveScript<T>() where T : Script, new()
        //{
        //    Type scriptType = typeof(T);
        //    if (!HasScript<T>())
        //    {
        //        Log.Error($"Script with name {scriptType.Name} does not exist on entity {Id}");
        //        return;
        //    }

        //    InternalCalls.Entity_RemoveScript(Id, GetScript<T>().scriptId);
        //}

        public T AddScript<T>() where T : Script, new()
        {
            Type componentType = typeof(T);

            var fullname = componentType.Namespace + "." + componentType.Name;
            InternalCalls.Entity_AddScript(Id, fullname, out ulong scriptId);

            List<ulong> newScriptsList = new List<ulong>();
            newScriptsList.AddRange(ScriptIds);
            newScriptsList.Add(scriptId);

            ScriptIds = newScriptsList.ToArray();

            return GetScript<T>();
        }

        public T GetScript<T>() where T : Script, new()
        {
            Type componentType = typeof(T);
            var fullname = componentType.Namespace + "." + componentType.Name;
            var instance = InternalCalls.Entity_GetScript(Id, fullname);

            if (!(instance is T))
            {
                Log.Error("Script is not of correct type!");
                return null;
            }

            return instance as T;
        }

        public T GetScriptInChildren<T>() where T : Script, new()
        {
            foreach (Entity child in children)
            {
                if (!child.HasScript<T>())
                {
                    continue;
                }

                return child.GetScript<T>();
            }
            return null;
        }
    }
}