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
