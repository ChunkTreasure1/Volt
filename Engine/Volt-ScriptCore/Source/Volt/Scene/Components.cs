﻿using System;
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

        public Quaternion rotation
        {
            get
            {
                InternalCalls.TransformComponent_GetRotation(entity.Id, out Quaternion rotation);
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

    public enum CollisionDetectionType : uint
    {
        Discrete = 0,
        Continuous,
        ContinuousSpeculative
    }

    public enum BodyType : uint
    {
        Static = 0,
        Dynamic
    }

    public enum ForceMode
    {
        Force,
        Impulse,
        VelocityChange,
        Acceleration
    }

    public class RigidbodyComponent : Component
    {
        public BodyType bodyType
        {
            get
            {
                return InternalCalls.RigidbodyComponent_GetBodyType(entity.Id);
            }

            set
            {
                InternalCalls.RigidbodyComponent_SetBodyType(entity.Id, ref value);
            }
        }

        public uint layerId
        {
            get
            {
                return InternalCalls.RigidbodyComponent_GetLayerId(entity.Id);
            }

            set
            {
                InternalCalls.RigidbodyComponent_SetLayerId(entity.Id, ref value);
            }
        }

        public float mass
        {
            get
            {
                return InternalCalls.RigidbodyComponent_GetMass(entity.Id);
            }

            set
            {
                InternalCalls.RigidbodyComponent_SetMass(entity.Id, ref value);
            }
        }

        public float linearDrag
        {
            get
            {
                return InternalCalls.RigidbodyComponent_GetLinearDrag(entity.Id);
            }

            set
            {
                InternalCalls.RigidbodyComponent_SetLinearDrag(entity.Id, ref value);
            }
        }

        public float angularDrag
        {
            get
            {
                return InternalCalls.RigidbodyComponent_GetAngularDrag(entity.Id);
            }

            set
            {
                InternalCalls.RigidbodyComponent_SetAngularDrag(entity.Id, ref value);
            }
        }

        public uint lockFlags
        {
            get
            {
                return InternalCalls.RigidbodyComponent_GetLockFlags(entity.Id);
            }

            set
            {
                InternalCalls.RigidbodyComponent_SetLockFlags(entity.Id, ref value);
            }
        }

        public bool disableGravity
        {
            get
            {
                return InternalCalls.RigidbodyComponent_GetDisableGravity(entity.Id);
            }

            set
            {
                InternalCalls.RigidbodyComponent_SetDisableGravity(entity.Id, ref value);
            }
        }

        public bool isKinematic
        {
            get
            {
                return InternalCalls.RigidbodyComponent_GetIsKinematic(entity.Id);
            }

            set
            {
                InternalCalls.RigidbodyComponent_SetIsKinematic(entity.Id, ref value);
            }
        }

        public void SetKinematicTarget(Vector3 position, Quaternion rotation)
        {
            InternalCalls.PhysicsActor_SetKinematicTarget(entity.Id, ref position, ref rotation);
        }

        public void SetLinearVelocity(Vector3 velocity)
        {
            InternalCalls.PhysicsActor_SetLinearVelocity(entity.Id, ref velocity);
        }

        public void SetAngularVelocity(Vector3 velocity)
        { 
            InternalCalls.PhysicsActor_SetAngularVelocity(entity.Id, ref velocity);
        }

        public void SetMaxLinearVelocity(float velocity)
        {
            InternalCalls.PhysicsActor_SetMaxLinearVelocity(entity.Id, ref velocity);
        }

        public void SetMaxAngularVelocity(float velocity)
        {
            InternalCalls.PhysicsActor_SetMaxAngularVelocity(entity.Id, ref velocity);
        }

        public Vector3 GetKinematicTargetPosition()
        {
            InternalCalls.PhysicsActor_GetKinematicTargetPosition(entity.Id, out Vector3 position);
            return position;
        }

        public Quaternion GetKinematicTargetRotation()
        {
            InternalCalls.PhysicsActor_GetKinematicTargetRotation(entity.Id, out Quaternion rotation);
            return rotation;
        }

        public void AddForce(Vector3 force, ForceMode forceMode)
        {
            InternalCalls.PhysicsActor_AddForce(entity.Id, ref force, forceMode);
        }

        public void AddTorque(Vector3 torque, ForceMode forceMode)
        { 
            InternalCalls.PhysicsActor_AddTorque(entity.Id, ref torque, forceMode);
        }

        public void WakeUp()
        { 
            InternalCalls.PhysicsActor_WakeUp(entity.Id);
        }

        public void PutToSleep()
        { 
            InternalCalls.PhysicsActor_PutToSleep(entity.Id);
        }
    }

    public class BoxColliderComponent : Component
    {
        public Vector3 halfSize
        {
            get
            {
                InternalCalls.BoxColliderComponent_GetHalfSize(entity.Id, out Vector3 halfSize);
                return halfSize;
            }

            set
            {
                InternalCalls.BoxColliderComponent_SetHalfSize(entity.Id, ref value);
            }
        }

        public Vector3 offset
        {
            get
            {
                InternalCalls.BoxColliderComponent_GetOffset(entity.Id, out Vector3 offset);
                return offset;
            }

            set
            {
                InternalCalls.BoxColliderComponent_SetOffset(entity.Id, ref value);
            }
        }

        public bool isTrigger
        {
            get
            {
                return InternalCalls.BoxColliderComponent_GetIsTrigger(entity.Id);
            }

            set
            {
                InternalCalls.BoxColliderComponent_SetIsTrigger(entity.Id, ref value);
            }
        }
    }

    public class SphereColliderComponent : Component
    {
        public float radius
        {
            get
            {
                return InternalCalls.SphereColliderComponent_GetRadius(entity.Id);
            }

            set
            {
                InternalCalls.SphereColliderComponent_SetRadius(entity.Id, ref value);
            }
        }

        public Vector3 offset
        {
            get
            {
                InternalCalls.SphereColliderComponent_GetOffset(entity.Id, out Vector3 offset);
                return offset;
            }

            set
            {
                InternalCalls.SphereColliderComponent_SetOffset(entity.Id, ref value);
            }
        }

        public bool isTrigger
        {
            get
            {
                return InternalCalls.SphereColliderComponent_GetIsTrigger(entity.Id);
            }

            set
            {
                InternalCalls.SphereColliderComponent_SetIsTrigger(entity.Id, ref value);
            }
        }
    }

    public class CapsuleColliderComponent : Component
    {
        public float radius
        {
            get
            {
                return InternalCalls.CapsuleColliderComponent_GetRadius(entity.Id);
            }

            set
            {
                InternalCalls.CapsuleColliderComponent_SetRadius(entity.Id, ref value);
            }
        }

        public float height
        {
            get
            {
                return InternalCalls.CapsuleColliderComponent_GetRadius(entity.Id);
            }

            set
            {
                InternalCalls.CapsuleColliderComponent_SetRadius(entity.Id, ref value);
            }
        }

        public Vector3 offset
        {
            get
            {
                InternalCalls.CapsuleColliderComponent_GetOffset(entity.Id, out Vector3 offset);
                return offset;
            }

            set
            {
                InternalCalls.CapsuleColliderComponent_SetOffset(entity.Id, ref value);
            }
        }

        public bool isTrigger
        {
            get
            {
                return InternalCalls.CapsuleColliderComponent_GetIsTrigger(entity.Id);
            }

            set
            {
                InternalCalls.CapsuleColliderComponent_SetIsTrigger(entity.Id, ref value);
            }
        }
    }

    public class MeshColliderComponent : Component
    {
        public bool isConvex
        {
            get
            {
                return InternalCalls.MeshColliderComponent_GetIsConvex(entity.Id);
            }

            set
            {
                InternalCalls.MeshColliderComponent_SetIsConvex(entity.Id, ref value);
            }
        }

        public bool isTrigger
        {
            get
            {
                return InternalCalls.MeshColliderComponent_GetIsTrigger(entity.Id);
            }

            set
            {
                InternalCalls.MeshColliderComponent_SetIsTrigger(entity.Id, ref value);
            }
        }

        public int subMeshIndex
        {
            get
            {
                return InternalCalls.MeshColliderComponent_GetSubMeshIndex(entity.Id);
            }

            set
            {
                InternalCalls.MeshColliderComponent_SetSubMeshIndex(entity.Id, ref value);
            }
        }
    }
}