using System;
using System.Runtime.CompilerServices;

namespace Volt
{
    internal static class InternalCalls
    {
        #region Entity
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(uint entityId, string componentType);
        #endregion

        #region TransformComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetPosition(uint entityId, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetPosition(uint entityId, ref Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetRotation(uint entityId, out Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetRotation(uint entityId, ref Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetScale(uint entityId, out Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetScale(uint entityId, ref Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetForward(uint entityId, out Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetRight(uint entityId, out Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetUp(uint entityId, out Vector3 scale);
        #endregion

        #region TagComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TagComponent_GetTag(uint entityId, out string tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TagComponent_SetTag(uint entityId, ref string tag);
        #endregion

        #region RelationshipComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RelationshipComponent_GetChildren(uint entityId, out Entity[] entities);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Entity RelationshipComponent_GetParent(uint entityId);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]

        internal extern static void RelationshipComponent_SetParent(uint entityId, ref Entity parentId);
        #endregion

        #region RigidbodyComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static BodyType RigidbodyComponent_GetBodyType(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetBodyType(uint entityId, ref BodyType bodyType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint RigidbodyComponent_GetLayerId(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetLayerId(uint entityId, ref uint layerId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float RigidbodyComponent_GetMass(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetMass(uint entityId, ref float mass);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float RigidbodyComponent_GetLinearDrag(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetLinearDrag(uint entityId, ref float linearDrag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint RigidbodyComponent_GetLockFlags(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetLockFlags(uint entityId, ref uint lockFlags);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float RigidbodyComponent_GetAngularDrag(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetAngularDrag(uint entityId, ref float angularDrag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool RigidbodyComponent_GetDisableGravity(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetDisableGravity(uint entityId, ref bool disableGravity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool RigidbodyComponent_GetIsKinematic(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetIsKinematic(uint entityId, ref bool isKinematic);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static CollisionDetectionType RigidbodyComponent_GetCollisionType(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponentSetCollisionType(uint entityId, ref CollisionDetectionType collisionType);
        #endregion

        #region BoxColliderComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxColliderComponent_GetHalfSize(uint entityId, out Vector3 halfSize);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxColliderComponent_SetHalfSize(uint entityId, ref Vector3 halfSize);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxColliderComponent_GetOffset(uint entityId, out Vector3 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxColliderComponent_SetOffset(uint entityId, ref Vector3 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool BoxColliderComponent_GetIsTrigger(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxColliderComponent_SetIsTrigger(uint entityId, ref bool isTrigger);
        #endregion

        #region SphereColliderComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float SphereColliderComponent_GetRadius(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SphereColliderComponent_SetRadius(uint entityId, ref float radius);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SphereColliderComponent_GetOffset(uint entityId, out Vector3 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SphereColliderComponent_SetOffset(uint entityId, ref Vector3 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SphereColliderComponent_GetIsTrigger(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SphereColliderComponent_SetIsTrigger(uint entityId, ref bool isTrigger);
        #endregion

        #region CapsuleColliderComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CapsuleColliderComponent_GetRadius(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CapsuleColliderComponent_SetRadius(uint entityId, ref float radius);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CapsuleColliderComponent_GetHeight(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CapsuleColliderComponent_SetHeight(uint entityId, ref float height);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CapsuleColliderComponent_GetOffset(uint entityId, out Vector3 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CapsuleColliderComponent_SetOffset(uint entityId, ref Vector3 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool CapsuleColliderComponent_GetIsTrigger(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CapsuleColliderComponent_SetIsTrigger(uint entityId, ref bool isTrigger);
        #endregion

        #region MeshColliderComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool MeshColliderComponent_GetIsConvex(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshColliderComponent_SetIsConvex(uint entityId, ref bool isConvex);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool MeshColliderComponent_GetIsTrigger(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshColliderComponent_SetIsTrigger(uint entityId, ref bool isTrigger);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int MeshColliderComponent_GetSubMeshIndex(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshColliderComponent_SetSubMeshIndex(uint entityId, ref int subMeshIndex);
        #endregion

        #region PhysicsActor
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_SetKinematicTarget(uint entityId, ref Vector3 position, ref Quaternion rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_SetLinearVelocity(uint entityId, ref Vector3 velocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_SetAngularVelocity(uint entityId, ref Vector3 velocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_SetMaxLinearVelocity(uint entityId, ref float velocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_SetMaxAngularVelocity(uint entityId, ref float velocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_GetKinematicTargetPosition(uint entityId, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_GetKinematicTargetRotation(uint entityId, out Quaternion rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_AddForce(uint entityId, ref Vector3 force, ForceMode forceMode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_AddTorque(uint entityId, ref Vector3 torque, ForceMode forceMode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_WakeUp(uint entityId);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_PutToSleep(uint entityId);
        #endregion

        #region Input
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_KeyDown(KeyCode keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_KeyUp(KeyCode keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_MouseDown(MouseButton keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_MouseUp(MouseButton keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_GetMousePosition();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_SetMousePosition(float x, float y);
        #endregion
    }
}
