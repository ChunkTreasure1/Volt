using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Volt
{
    public abstract class Component
    {
        public Entity entity { get; internal set; }
    }

    public class TransformComponent : Component
    {
        public Vector3 position
        {
            get
            {
                InternalCalls.TransformComponent_GetPosition(entity.Id, out Vector3 position);
                return position;
            }

            set 
            { 
                InternalCalls.TransformComponent_SetPosition(entity.Id, ref value);
            }
        }

        public Vector3 rotation
        {
            get
            {
                InternalCalls.TransformComponent_GetRotation(entity.Id, out Vector3 rotation);
                return rotation;
            }

            set
            {
                InternalCalls.TransformComponent_SetRotation(entity.Id, ref value);
            }
        }

        public Vector3 scale
        {
            get
            {
                InternalCalls.TransformComponent_GetScale(entity.Id, out Vector3 scale);
                return scale;
            }

            set
            {
                InternalCalls.TransformComponent_SetScale(entity.Id, ref value);
            }
        }

        public Vector3 forward
        {
            get
            {
                InternalCalls.TransformComponent_GetForward(entity.Id, out Vector3 forward);
                return forward;
            }

            private set { }
        }

        public Vector3 right
        {
            get
            {
                InternalCalls.TransformComponent_GetRight(entity.Id, out Vector3 right);
                return right;
            }

            private set { }
        }

        public Vector3 up
        {
            get
            {
                InternalCalls.TransformComponent_GetUp(entity.Id, out Vector3 up);
                return up;
            }

            private set { }
        }
    }

    public class TagComponent : Component
    {
        public string tag 
        { 
            get
            {
                InternalCalls.TagComponent_GetTag(entity.Id, out string tag);
                return tag;
            }

            set
            {
                InternalCalls.TagComponent_SetTag(entity.Id, ref value);
            }
        }
    }

    public class RelationshipComponent : Component
    {
        //public Entity[] children
        //{
        //    private get { ret }
        //    private set { }
        //}

        public Entity parent
        {
            get
            {
                return InternalCalls.RelationshipComponent_GetParent(entity.Id);
            }

            set
            {
                InternalCalls.RelationshipComponent_SetParent(entity.Id, ref value);
            }
        }
    }
}
