namespace Volt
{

    // #TODO_Ivar: Add guids to all components
    public enum VideoStatus : uint
    {
        Stopped = 0,
        Playing,
        End
    }

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

        public Vector3 localPosition
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalPosition(entity.Id, out Vector3 position);
                return position;
            }

            set
            {
                InternalCalls.TransformComponent_SetLocalPosition(entity.Id, ref value);
            }
        }

        public Quaternion localRotation
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalRotation(entity.Id, out Quaternion rotation);
                return rotation;
            }

            set
            {
                InternalCalls.TransformComponent_SetLocalRotation(entity.Id, ref value);
            }
        }

        public Vector3 localScale
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalScale(entity.Id, out Vector3 scale);
                return scale;
            }

            set
            {
                InternalCalls.TransformComponent_SetLocalScale(entity.Id, ref value);
            }
        }

        public Vector3 localForward
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalForward(entity.Id, out Vector3 forward);
                return forward;
            }

            private set { }
        }

        public Vector3 localRight
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalRight(entity.Id, out Vector3 right);
                return right;
            }

            private set { }
        }

        public Vector3 localUp
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalUp(entity.Id, out Vector3 up);
                return up;
            }

            private set { }
        }

        public bool visible
        {
            get
            {
                return InternalCalls.TransformComponent_GetVisible(entity.Id);
            }

            set
            {
                InternalCalls.TransformComponent_SetVisible(entity.Id, value);
            }
        }
    }

    public class TagComponent : Component
    {
        public string tag
        {
            get
            {
                return InternalCalls.TagComponent_GetTag(entity.Id);
            }

            set
            {
                InternalCalls.TagComponent_SetTag(entity.Id, ref value);
            }
        }
    }

    public class RelationshipComponent : Component
    {
        public Entity[] children
        {
            get
            {
                var result = InternalCalls.RelationshipComponent_GetChildren(entity.Id);
                if (result == null)
                {
                    return new Entity[0];
                }
                return result;
            }
        }

        public Entity parent
        {
            get
            {
                return InternalCalls.RelationshipComponent_GetParent(entity.Id);
            }

            set
            {
                if (object.ReferenceEquals(value, null))
                {
                    InternalCalls.RelationshipComponent_SetParent(entity.Id, 0);
                }
                else
                {
                    InternalCalls.RelationshipComponent_SetParent(entity.Id, value.Id);
                }
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

    public enum ForceMode : uint
    {
        Force,
        Impulse,
        VelocityChange,
        Acceleration
    }

    public enum ClimbingMode : uint
    {
        Normal,
        Constrained
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

        public CollisionDetectionType collisionType
        {
            get
            {
                return InternalCalls.RigidbodyComponent_GetCollisionDetectionType(entity.Id);
            }

            set
            {
                InternalCalls.RigidbodyComponent_SetCollisionDetectionType(entity.Id, ref value);
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

        public Vector3 linearVelocity
        {
            get
            {
                InternalCalls.PhysicsActor_GetLinearVelocity(entity.Id, out Vector3 v);
                return v;
            }

            set
            {
                InternalCalls.PhysicsActor_SetLinearVelocity(entity.Id, ref value);
            }
        }

        public Vector3 angularVelocity
        {
            get
            {
                InternalCalls.PhysicsActor_GetAngularVelocity(entity.Id, out Vector3 v);
                return v;
            }

            set
            {
                InternalCalls.PhysicsActor_SetAngularVelocity(entity.Id, ref value);
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
        public Mesh colliderMesh
        {
            get
            {
                return AssetManager.GetAsset<Mesh>(InternalCalls.MeshColliderComponent_GetColliderMesh(entity.Id));
            }

            set
            {
                if (value == null)
                {
                    InternalCalls.MeshColliderComponent_SetColliderMesh(entity.Id, 0);
                }
                else
                {
                    InternalCalls.MeshColliderComponent_SetColliderMesh(entity.Id, value.handle);
                }
            }
        }

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

    public class CharacterControllerComponent : Component
    {
        public float slopeLimit
        {
            get
            {
                return InternalCalls.CharacterControllerComponent_GetSlopeLimit(entity.Id);
            }

            set
            {
                InternalCalls.CharacterControllerComponent_SetSlopeLimit(entity.Id, value);
            }
        }

        public float invisibleWallHeight
        {
            get
            {
                return InternalCalls.CharacterControllerComponent_GetInvisibleWallHeight(entity.Id);
            }

            set
            {
                InternalCalls.CharacterControllerComponent_SetInvisibleWallHeight(entity.Id, value);
            }
        }

        public float maxJumpHeight
        {
            get
            {
                return InternalCalls.CharacterControllerComponent_GetMaxJumpHeight(entity.Id);
            }

            set
            {
                InternalCalls.CharacterControllerComponent_SetMaxJumpHeight(entity.Id, value);
            }
        }

        public float contactOffset
        {
            get
            {
                return InternalCalls.CharacterControllerComponent_GetContactOffset(entity.Id);
            }

            set
            {
                InternalCalls.CharacterControllerComponent_SetContactOffset(entity.Id, value);
            }
        }

        public float stepOffset
        {
            get
            {
                return InternalCalls.CharacterControllerComponent_GetStepOffset(entity.Id);
            }

            set
            {
                InternalCalls.CharacterControllerComponent_SetStepOffset(entity.Id, value);
            }
        }

        public float density
        {
            get
            {
                return InternalCalls.CharacterControllerComponent_GetDensity(entity.Id);
            }

            set
            {
                InternalCalls.CharacterControllerComponent_SetDensity(entity.Id, value);
            }
        }

        public float height
        {
            get
            {
                return InternalCalls.PhysicsControllerActor_GetHeight(entity.Id);
            }

            set
            {
                InternalCalls.PhysicsControllerActor_SetHeight(entity.Id, value);
            }
        }

        public float radius
        {
            get
            {
                return InternalCalls.PhysicsControllerActor_GetRadius(entity.Id);
            }

            set
            {
                InternalCalls.PhysicsControllerActor_SetRadius(entity.Id, value);
            }
        }

        public float gravity
        {
            get
            {
                return InternalCalls.CharacterControllerComponent_GetGravity(entity.Id);
            }

            set
            {
                InternalCalls.CharacterControllerComponent_SetGravity(entity.Id, value);
            }
        }

        public bool isGrounded
        {
            get
            {
                return InternalCalls.PhysicsControllerActor_IsGrounded(entity.Id);
            }

            private set { }
        }

        public Vector3 angularVelocity
        {
            get
            {
                InternalCalls.CharacterControllerComponent_GetAngularVelocity(entity.Id, out Vector3 v);
                return v;
            }

            set
            {
                InternalCalls.CharacterControllerComponent_SetAngularVelocity(entity.Id, ref value);
            }
        }
        public Vector3 linearVelocity
        {
            get
            {
                InternalCalls.CharacterControllerComponent_GetLinearVelocity(entity.Id, out Vector3 v);
                return v;
            }

            set
            {
                InternalCalls.CharacterControllerComponent_SetLinearVelocity(entity.Id, ref value);
            }
        }

        public void Move(Vector3 velocity)
        {
            InternalCalls.PhysicsControllerActor_Move(entity.Id, ref velocity);
        }

        public void Jump(float jumpForce)
        {
            InternalCalls.PhysicsControllerActor_Jump(entity.Id, jumpForce);
        }

        public void SetPosition(Vector3 position)
        {
            InternalCalls.PhysicsControllerActor_SetPosition(entity.Id, ref position);
        }

        public void SetFootPosition(Vector3 position)
        {
            InternalCalls.PhysicsControllerActor_SetFootPosition(entity.Id, ref position);
        }

        public Vector3 GetFootPosition()
        {
            InternalCalls.PhysicsControllerActor_GetFootPosition(entity.Id, out Vector3 position);
            return position;
        }

        public Vector3 GetPosition()
        {
            InternalCalls.PhysicsControllerActor_GetPosition(entity.Id, out Vector3 position);
            return position;
        }
    }

    public class VisualScriptingComponent : Component
    { }

    public class AnimationControllerComponent : Component
    {
        private AnimationController myController;

        public AnimationController controller
        {
            get
            {
                if (myController == null)
                {
                    myController = new AnimationController(entity);
                }

                return myController;
            }
            set { }
        }

        public BoundingSphere boundingSphere
        {
            get
            {
                BoundingSphere sphere;
                InternalCalls.AnimationControllerComponent_GetBoundingSphere(entity.Id, out sphere.center, out sphere.radius);
                return sphere;
            }
        }

        public Vector3 rootMotion
        {
            get
            {
                InternalCalls.AnimationControllerComponent_GetRootMotion(entity.Id, out Vector3 rootMotion);
                return rootMotion;
            }
            private set { }
        }

        public Material material
        {
            private get
            {
                return AssetManager.GetAsset<Material>(InternalCalls.AnimationControllerComponent_GetOverrideMaterial(entity.Id));
            }

            set
            {
                if (value == null)
                {
                    InternalCalls.AnimationControllerComponent_SetOverrideMaterial(entity.Id, 0);
                }
                else
                {
                    InternalCalls.AnimationControllerComponent_SetOverrideMaterial(entity.Id, value.handle);
                }
            }
        }

        public Mesh overrideSkin
        {
            get
            {
                return AssetManager.GetAsset<Mesh>(InternalCalls.AnimationControllerComponent_GetOverrideSkin(entity.Id));
            }

            set
            {
                if (value == null)
                {
                    InternalCalls.AnimationControllerComponent_SetOverrideSkin(entity.Id, 0);
                }
                else
                {
                    InternalCalls.AnimationControllerComponent_SetOverrideSkin(entity.Id, value.handle);
                }
            }
        }
    }

    public class TextRendererComponent : Component
    {
        public string text
        {
            get
            {
                InternalCalls.TextRendererComponent_GetText(entity.Id, out string text);
                return text;
            }
            set { InternalCalls.TextRendererComponent_SetText(entity.Id, ref value); }
        }

        public float maxWidth
        {
            get { return InternalCalls.TextRendererComponent_GetMaxWidth(entity.Id); }
            set { InternalCalls.TextRendererComponent_SetMaxWidth(entity.Id, value); }
        }

        public Vector4 color
        {
            get
            {
                InternalCalls.TextRendererComponent_GetColor(entity.Id, out Vector4 color);
                return color;
            }
            set { InternalCalls.TextRendererComponent_SetColor(entity.Id, ref value); }
        }
    }

    public class MeshComponent : Component
    {
        public Mesh mesh
        {
            get
            {
                Mesh mesh = AssetManager.GetAsset<Mesh>(InternalCalls.MeshComponent_GetMeshHandle(entity.Id));
                if (mesh == null)
                {
                    return null;
                }

                return mesh;
            }

            set 
            {
                InternalCalls.MeshComponent_SetMeshHandle(entity.Id, value.handle);
            }
        }

        public Material material
        {
            get
            {
                if (!InternalCalls.MeshComponent_HasOverrideMaterial(entity.Id))
                {
                    return mesh.material;
                }

                return AssetManager.GetAsset<Material>(InternalCalls.MeshComponent_GetOverrideMaterial(entity.Id));
            }

            set
            {
                if (value == null)
                {
                    InternalCalls.MeshComponent_SetOverrideMaterial(entity.Id, 0);
                }
                else
                {
                    InternalCalls.MeshComponent_SetOverrideMaterial(entity.Id, value.handle);
                }
            }
        }
    }

    public enum ObstacleAvoidanceQuality : uint
    {
        None = 0,
        Low,
        Medium,
        High
    }

    public class NavAgentComponent : Component
    {
        public bool active
        {
            get
            {
                return InternalCalls.NavAgentComponent_GetActive(entity.Id);
            }
            set
            {
                InternalCalls.NavAgentComponent_SetActive(entity.Id, value);
            }
        }

        public Vector3 position
        {
            get
            {
                InternalCalls.NavAgentComponent_GetPosition(entity.Id, out Vector3 position);
                return target;
            }
            set
            {
                InternalCalls.NavAgentComponent_SetPosition(entity.Id, ref value);
            }
        }

        public Vector3 target
        {
            get
            {
                InternalCalls.NavAgentComponent_GetTarget(entity.Id, out Vector3 target);
                return target;
            }
            set
            {
                InternalCalls.NavAgentComponent_SetTarget(entity.Id, ref value);
            }
        }

        public Vector3 velocity
        {
            get
            {
                InternalCalls.NavAgentComponent_GetVelocity(entity.Id, out Vector3 velocity);
                return velocity;
            }
        }

        public float radius
        {
            get
            {
                return InternalCalls.NavAgentComponent_GetRadius(entity.Id);
            }
            set
            {
                InternalCalls.NavAgentComponent_SetRadius(entity.Id, value);
            }
        }

        public float height
        {
            get
            {
                return InternalCalls.NavAgentComponent_GetHeight(entity.Id);
            }
            set
            {
                InternalCalls.NavAgentComponent_SetHeight(entity.Id, value);
            }
        }

        public float maxSpeed
        {
            get
            {
                return InternalCalls.NavAgentComponent_GetMaxSpeed(entity.Id);
            }
            set
            {
                InternalCalls.NavAgentComponent_SetMaxSpeed(entity.Id, value);
            }
        }

        public float acceleration
        {
            get
            {
                return InternalCalls.NavAgentComponent_GetAcceleration(entity.Id);
            }
            set
            {
                InternalCalls.NavAgentComponent_SetAcceleration(entity.Id, value);
            }
        }

        public float separationWeight
        {
            get
            {
                return InternalCalls.NavAgentComponent_GetSeperationWeight(entity.Id);
            }
            set
            {
                InternalCalls.NavAgentComponent_SetSeperationWeight(entity.Id, value);
            }
        }

        public ObstacleAvoidanceQuality obstacleAvoidanceQuality
        {
            get
            {
                return InternalCalls.NavAgentComponent_GetObstacleAvoidanceQuality(entity.Id);
            }
            set
            {
                InternalCalls.NavAgentComponent_SetObstacleAvoidanceQuality(entity.Id, value);
            }
        }
    }
}
