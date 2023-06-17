using System;
using System.Runtime.CompilerServices;

namespace Volt
{
    internal static class InternalCalls
    {
        #region VoltApplication

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VoltApplication_LoadLevel(string aLevelPath);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VoltApplication_SetResolution(uint aResolutionX, uint aResolutionY);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VoltApplication_SetWindowMode(uint aWindowMode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool VoltApplication_IsRuntime();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VoltApplication_Quit();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string VoltApplication_GetClipboard();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VoltApplication_SetClipboard(string text);

        #endregion

        #region GraphKey
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GraphKey_DispatchEvent(uint entity, string eventName);
        #endregion

        #region Entity
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_IsValid(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(uint entityId, string componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasScript(uint entityId, string script);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Entity_GetScript(uint entityId, string script);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_RemoveComponent(uint entityId, string componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_RemoveScript(uint entityId, ulong scriptId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_AddComponent(uint entityId, string componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_AddScript(uint entityId, string componentType, out ulong scriptId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Entity_FindByName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Entity_FindById(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Entity_CreateNewEntity(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Entity_CreateNewEntityWithPrefab(ulong handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_DeleteEntity(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Entity_Clone(uint id);

        #endregion

        #region Scene 

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Scene_Load(ulong handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Scene_Preload(string scenePath);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Scene_Save(ulong handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Entity[] Scene_GetAllEntities();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Entity[] Scene_GetAllEntitiesWithComponent(string componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Entity[] Scene_GetAllEntitiesWithScript(string scriptType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Entity Scene_InstantiateSplitMesh(ulong meshHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Scene_CreateDynamicPhysicsActor(uint entityId);
        #endregion

        #region TransformComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetPosition(uint entityId, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetPosition(uint entityId, ref Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetRotation(uint entityId, out Quaternion rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetRotation(uint entityId, ref Quaternion rotation);

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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetLocalPosition(uint entityId, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetLocalPosition(uint entityId, ref Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetLocalRotation(uint entityId, out Quaternion rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetLocalRotation(uint entityId, ref Quaternion rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetLocalScale(uint entityId, out Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetLocalScale(uint entityId, ref Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetLocalForward(uint entityId, out Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetLocalRight(uint entityId, out Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetLocalUp(uint entityId, out Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool TransformComponent_GetVisible(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetVisible(uint entityId, bool visible);
        #endregion

        #region TagComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string TagComponent_GetTag(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TagComponent_SetTag(uint entityId, ref string tag);
        #endregion

        #region RelationshipComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Entity[] RelationshipComponent_GetChildren(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object RelationshipComponent_FindByName(uint entityId, string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Entity RelationshipComponent_GetParent(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RelationshipComponent_SetParent(uint entityId, uint parentId);
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
        internal extern static CollisionDetectionType RigidbodyComponent_GetCollisionDetectionType(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetCollisionDetectionType(uint entityId, ref CollisionDetectionType collisionType);
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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong MeshColliderComponent_GetColliderMesh(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshColliderComponent_SetColliderMesh(uint entityId, ulong meshHandle);
        #endregion

        #region CharacterControllerComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CharacterControllerComponent_GetSlopeLimit(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CharacterControllerComponent_SetSlopeLimit(uint entityId, float slopeLimit);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CharacterControllerComponent_GetInvisibleWallHeight(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CharacterControllerComponent_SetInvisibleWallHeight(uint entityId, float height);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CharacterControllerComponent_GetMaxJumpHeight(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CharacterControllerComponent_SetMaxJumpHeight(uint entityId, float maxJumpHeight);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CharacterControllerComponent_GetContactOffset(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CharacterControllerComponent_SetContactOffset(uint entityId, float contactOffset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CharacterControllerComponent_GetStepOffset(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CharacterControllerComponent_SetStepOffset(uint entityId, float stepOffset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CharacterControllerComponent_GetDensity(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CharacterControllerComponent_SetDensity(uint entityId, float density);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CharacterControllerComponent_GetGravity(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CharacterControllerComponent_SetGravity(uint entityId, float gravity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CharacterControllerComponent_GetAngularVelocity(uint entityId, out Vector3 angularVelocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CharacterControllerComponent_SetAngularVelocity(uint entityId, ref Vector3 angularVelocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CharacterControllerComponent_GetLinearVelocity(uint entityId, out Vector3 linearVelocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CharacterControllerComponent_SetLinearVelocity(uint entityId, ref Vector3 linearVelocity);

        #endregion

        #region VisualScriptingComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VisualScriptingComponent_SetParameterFloat(uint entityId, string name, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VisualScriptingComponent_SetParameterInt(uint entityId, string name, int value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VisualScriptingComponent_SetParameterBool(uint entityId, string name, bool value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VisualScriptingComponent_SetParameterVector3(uint entityId, string name, ref Vector3 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VisualScriptingComponent_SetParameterString(uint entityId, string name, ref string value);
        #endregion

        #region AnimationControllerComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_SetParameterFloat(uint entityId, string name, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_SetParameterInt(uint entityId, string name, int value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_SetParameterBool(uint entityId, string name, bool value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_SetParameterVector3(uint entityId, string name, ref Vector3 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_SetParameterString(uint entityId, string name, ref string value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_GetBoundingSphere(uint entityId, out Vector3 center, out float radius);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float AnimationControllerComponent_GetParameterFloat(uint entityId, string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int AnimationControllerComponent_GetParameterInt(uint entityId, string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AnimationControllerComponent_GetParameterBool(uint entityId, string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Vector3 AnimationControllerComponent_GetParameterVector3(uint entityId, string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string AnimationControllerComponent_GetParameterString(uint entityId, string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AnimationControllerComponent_GetHighlighted(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_SetHighlighted(uint entityId, bool state);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_GetHighlightedColor(uint entityId, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_SetHighlightedColor(uint entityId, ref Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_GetRootMotion(uint entityId, out Vector3 rootMotion);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_AttachEntity(string attachmentName, uint entityId, uint attachEntity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_DetachEntity(uint entityId, uint attachEntity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AnimationControllerComponent_HasOverrideMaterial(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_SetOverrideMaterial(uint entityId, ulong materialHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong AnimationControllerComponent_GetOverrideMaterial(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_SetOverrideSkin(uint entityId, ulong skinHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationControllerComponent_SetController(uint entityId, ulong animGraphHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong AnimationControllerComponent_GetOverrideSkin(uint entityId);
        #endregion

        #region TextRendererComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_GetText(uint entityId, out string text);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetText(uint entityId, ref string text);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float TextRendererComponent_GetMaxWidth(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetMaxWidth(uint entityId, float width);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_GetColor(uint entityId, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetColor(uint entityId, ref Vector4 color);
        #endregion

        #region MeshComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong MeshComponent_GetMeshHandle(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshComponent_SetMeshHandle(uint entityId, ulong handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool MeshComponent_HasOverrideMaterial(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshComponent_SetOverrideMaterial(uint entityId, ulong materialHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong MeshComponent_GetOverrideMaterial(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool MeshComponent_GetHighlighted(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshComponent_SetHighlighted(uint entityId, bool state);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshComponent_GetHighlightedColor(uint entityId, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshComponent_SetHighlightedColor(uint entityId, ref Vector4 color);
        #endregion

        #region SpotlightComponent

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotlightComponent_GetColor(uint entityId, out Vector3 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotlightComponent_SetColor(uint entityId, ref Vector3 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float SpotlightComponent_GetIntensity(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotlightComponent_SetIntensity(uint entityId, float active);

        #endregion

        #region PointlightComponent

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PointlightComponent_GetColor(uint entityId, out Vector3 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PointlightComponent_SetColor(uint entityId, ref Vector3 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float PointlightComponent_GetIntensity(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PointlightComponent_SetIntensity(uint entityId, float active);

        #endregion

        #region Mesh
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Mesh_GetMaterial(ulong meshHandle);
        #endregion

        #region Material
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetBool(ulong handle, string name, bool value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetInt(ulong handle, string name, int value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetUInt(ulong handle, string name, uint value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetFloat(ulong handle, string name, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetFloat2(ulong handle, string name, ref Vector2 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetFloat3(ulong handle, string name, ref Vector3 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetFloat4(ulong handle, string name, ref Vector4 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Material_CreateCopy(ulong materialToCopy);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetColor(ulong handle, ref Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_GetColor(ulong handle, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetEmissiveColor(ulong handle, ref Vector3 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_GetEmissiveColor(ulong handle, out Vector3 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetEmissiveStrength(ulong handle, float strength);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Material_GetEmissiveStrength(ulong handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetRoughness(ulong handle, float roughness);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Material_GetRoughness(ulong handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetMetalness(ulong handle, float metalness);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Material_GetMetalness(ulong handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetNormalStrength(ulong handle, float normalStrength);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Material_GetNormalStrength(ulong handle);
        #endregion

        #region PostProcessingMaterial
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PostProcessingMaterial_SetBool(ulong handle, string name, bool value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PostProcessingMaterial_SetInt(ulong handle, string name, int value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PostProcessingMaterial_SetUInt(ulong handle, string name, uint value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PostProcessingMaterial_SetFloat(ulong handle, string name, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PostProcessingMaterial_SetFloat2(ulong handle, string name, ref Vector2 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PostProcessingMaterial_SetFloat3(ulong handle, string name, ref Vector3 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PostProcessingMaterial_SetFloat4(ulong handle, string name, ref Vector4 value);
        #endregion

        #region Animation
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Animation_GetDuration(ulong handle);
        #endregion

        #region PhysicsActor
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_SetKinematicTarget(uint entityId, ref Vector3 position, ref Quaternion rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_SetLinearVelocity(uint entityId, ref Vector3 velocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_SetAngularVelocity(uint entityId, ref Vector3 velocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_GetLinearVelocity(uint entityId, out Vector3 velocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsActor_GetAngularVelocity(uint entityId, out Vector3 velocity);

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

        #region PhysicsControllerActor
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float PhysicsControllerActor_GetHeight(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsControllerActor_SetHeight(uint entityId, float height);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float PhysicsControllerActor_GetRadius(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsControllerActor_SetRadius(uint entityId, float radius);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsControllerActor_Move(uint entityId, ref Vector3 velocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsControllerActor_SetPosition(uint entityId, ref Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsControllerActor_SetFootPosition(uint entityId, ref Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsControllerActor_GetPosition(uint entityId, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsControllerActor_GetFootPosition(uint entityId, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool PhysicsControllerActor_IsGrounded(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PhysicsControllerActor_Jump(uint entityId, float jumpForce);

        #endregion

        #region Physics
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Physics_GetGravity(out Vector3 outGravity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Physics_SetGravity(ref Vector3 gravity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Physics_Raycast(ref Vector3 origin, ref Vector3 direction, out InternalRaycastHit hit, float maxDistance);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Physics_RaycastLayerMask(ref Vector3 origin, ref Vector3 direction, out InternalRaycastHit hit, float maxDistance, uint layerMask);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Entity[] Physics_OverlapBox(ref Vector3 origin, ref Vector3 halfSize, uint layerMask);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Entity[] Physics_OverlapSphere(ref Vector3 origin, float radius, uint layerMask);
        #endregion

        #region InputMapper
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int InputMapper_GetKey(string key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void InputMapper_SetKey(string key, int value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void InputMapper_ResetKey(string key);

        #endregion

        #region Input
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_KeyPressed(KeyCode keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint[] Input_GetAllKeyPressed();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_KeyReleased(KeyCode keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_MousePressed(MouseButton keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_MouseReleased(MouseButton keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_KeyDown(KeyCode keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_KeyUp(KeyCode keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_MouseDown(MouseButton keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_MouseUp(MouseButton keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetMousePosition(out Vector2 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_SetMousePosition(float x, float y);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_ShowCursor(bool state);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Input_GetScrollOffset();
        #endregion

        #region Log
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Log_String(string text, LogLevel logLevel);
        #endregion

        #region Noise
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Noise_Perlin(float x, float y, float z);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Noise_SetSeed(int seed);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Noise_SetFrequency(float frequency);
        #endregion

        #region AMP

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AMP_PlayOneshotEvent(string aString);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AMP_SetRTPC(string aRTPCName, int aValue);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AMP_StopAllEvents();

        //EVENT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint AudioSourceComponent_PlayEvent(uint ID, string aString);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AudioSourceComponent_PlayOneshotEvent(uint ID, string aEventName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AudioSourceComponent_StopEvent(uint ID, uint aPlayingID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AudioSourceComponent_PauseEvent(uint ID, uint aPlayingID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AudioSourceComponent_ResumeEvent(uint ID, uint aPlayingID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSourceComponent_StopAllEvent(uint ID);

        //GAME SYNCS

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AudioSourceComponent_SetState(uint ID, string aStateGroup, string aState);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AudioSourceComponent_SetSwitch(uint ID, string aSwitchGroup, string aState);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AudioSourceComponent_SetParameter(uint ID, string aParameterName, float aValue);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AudioSourceComponent_SetParameterOverTime(uint ID, string aParameterName, float aValue, uint aOvertime);

        #endregion

        #region Vision

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Vision_GetActiveCamera();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Vision_SetActiveCamera(uint id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Vision_DoCameraShake(uint id, ref Vision.CameraShakeSettings setting);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Vision_SetCameraFollow(uint camId, uint followId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Vision_SetCameraLookAt(uint camId, uint lookAtId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Vision_SetCameraFocusPoint(uint camId, uint focusId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Vision_SetCameraFollowConstraints(uint camId, bool X, bool Y, bool Z);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Vision_SetCameraDampAmount(uint camId, float dampAmount);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Vision_SetCameraFieldOfView(uint camId, float fov);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Vision_SetCameraLocked(uint camId, bool locked);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Vision_SetCameraMouseSensentivity(uint camId, float mouseSens);

        #endregion

        #region Render
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DebugRenderer_DrawLine(ref Vector3 startPosition, ref Vector3 endPosition, ref Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DebugRenderer_DrawSprite(ref Vector3 position, ref Vector3 rotation, ref Vector3 scale, ref Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DebugRenderer_DrawText(ref string text, ref Vector3 position, ref Vector3 rotation, ref Vector3 scale);
        #endregion

        #region Navigation

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Vector3[] Navigation_FindPath(ref Vector3 start, ref Vector3 end, ref Vector3 polygonSearchDistance);

        #endregion

        #region NavAgentComponent

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NavAgentComponent_GetTarget(uint entityId, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NavAgentComponent_SetTarget(uint entityId, ref Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NavAgentComponent_GetPosition(uint entityId, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NavAgentComponent_SetPosition(uint entityId, ref Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool NavAgentComponent_GetActive(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NavAgentComponent_SetActive(uint entityId, bool active);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NavAgentComponent_GetVelocity(uint entityId, out Vector3 velocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float NavAgentComponent_GetRadius(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NavAgentComponent_SetRadius(uint entityId, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float NavAgentComponent_GetHeight(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NavAgentComponent_SetHeight(uint entityId, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float NavAgentComponent_GetMaxSpeed(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NavAgentComponent_SetMaxSpeed(uint entityId, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float NavAgentComponent_GetAcceleration(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NavAgentComponent_SetAcceleration(uint entityId, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float NavAgentComponent_GetSeperationWeight(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NavAgentComponent_SetSeperationWeight(uint entityId, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ObstacleAvoidanceQuality NavAgentComponent_GetObstacleAvoidanceQuality(uint entityId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NavAgentComponent_SetObstacleAvoidanceQuality(uint entityId, ObstacleAvoidanceQuality value);

        #endregion

        #region AssetManager
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AssetManager_HasAsset(ulong assetHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong AssetManager_GetAssetHandleFromPath(string path);
        #endregion

        #region Asset
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Asset_IsValid(ulong assetHandle);
        #endregion

        #region Timer
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_GetDeltaTime();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_SetTimeScale(float timeScale);
        #endregion

        #region Project
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Project_GetDirectory();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Project_GetProjectName();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Project_GetCompanyName();
        #endregion

        #region UIRenderer
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void UIRenderer_SetViewport(float x, float y, float width, float height);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void UIRenderer_SetScissor(int x, int y, uint width, uint height);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void UIRenderer_DrawSprite(ref Vector3 position, ref Vector2 scale, float rotation, ref Vector4 color, ref Vector2 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void UIRenderer_DrawSpriteTexture(ulong textureHandle, ref Vector3 position, ref Vector2 scale, float rotation, ref Vector4 color, ref Vector2 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void UIRenderer_DrawString(string text, ulong fontHandle, ref Vector3 position, ref Vector2 scale, float rotation, float maxWidth, ref Vector4 color, ref Vector2 positionOffset);
        #endregion

        #region Net
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static UInt16 NetActorComponent_GetRepId(uint entityId);

        // Event
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NetEvent_TriggerEventFromLocalId(uint repId, eNetEvent netEvent, byte[] args);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NetEvent_TriggerEventFromNetId(UInt64 repId, eNetEvent netEvent, byte[] args);

        // Scene interactions
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NetScene_DestroyFromNetId(UInt64 repId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NetScene_DestroyFromLocalId(uint repId);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NetScene_InstantiatePrefabAtEntity(UInt64 handle, uint spawnPoint);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Net_Notify(uint repId, string fieldName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Net_NotifyFromNetId(UInt64 repId, string fieldName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Net_SceneLoad();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Net_StartClient();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Net_StartServer();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Net_StartSinglePlayer();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Net_SetHandleTick(bool value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Net_Connect(string ip, ushort port);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Net_Disconnect();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Net_InstantiatePlayer();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ushort Net_GetBoundPort();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ushort Net_ForcePortBinding(ushort port);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ushort Net_Reload();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Net_IsHost();
         
        #endregion

        #region SteamAPI

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SteamAPI_StartLobby(string address);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SteamAPI_SetStatInt(string name, int value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SteamAPI_SetStatFloat(string name, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int SteamAPI_GetStatInt(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float SteamAPI_GetStatFloat(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SteamAPI_IsStatsValid();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SteamAPI_StoreStats();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SteamAPI_RequestStats();
        #endregion

        #region Window
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Window_GetWidth();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Window_GetHeight();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetCursor(string path);
        #endregion

        #region Font
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Font_GetStringWidth(ulong fontHandle, string text, ref Vector2 scale, float maxWidth);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Font_GetStringHeight(ulong fontHandle, string text, ref Vector2 scale, float maxWidth);
        #endregion

        #region PostProcessingStack
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PostProcessingStack_PushEffect(ulong effectHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PostProcessingStack_PopEffect();
        #endregion

        #region DiscordSDK

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DiscordSDK_Init(long appId);

        #endregion

        #region Renderer
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Renderer_SetRenderScale(float renderScale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Renderer_SetRendererSettings(ref RendererSettings settings);

        #endregion

        #region VideoPlayer
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VideoPlayer_Play(ulong assetHandle, bool loop);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VideoPlayer_Stop(ulong assetHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VideoPlayer_Restart(ulong assetHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VideoPlayer_Pause(ulong assetHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VideoPlayer_Update(ulong assetHandle, float deltaTime);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void VideoPlayer_DrawSpriteTexture(ulong videoHandle, ref Vector3 position, ref Vector2 scale, float rotation, ref Vector4 color, ref Vector2 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static VideoStatus VideoPlayer_GetStatus(ulong videoHandle);

        #endregion
    }
}
