namespace Volt
{
    public struct RaycastHit
    {
        public Entity entity;
        public Vector3 position;
        public Vector3 normal;
        public float distance;
    }

    internal struct InternalRaycastHit
    {
        public uint entityId;
        public Vector3 position;
        public Vector3 normal;
        public float distance;
    }

    public class Physics
    {
        static public Vector3 gravity
        {
            get
            {
                InternalCalls.Physics_GetGravity(out Vector3 v);
                return v;
            }
            set
            {
                InternalCalls.Physics_SetGravity(ref value);
            }
        }

        static public bool Raycast(Vector3 origin, Vector3 direction, out RaycastHit hit, float maxDistance)
        {
            var intHit = new InternalRaycastHit();
            bool result = InternalCalls.Physics_Raycast(ref origin, ref direction, out intHit, maxDistance);

            hit.entity = InternalCalls.Entity_FindById(intHit.entityId) as Entity;
            hit.position = intHit.position;
            hit.normal = intHit.normal;
            hit.distance = intHit.distance;

            return result;
        }

        static public bool Raycast(Vector3 origin, Vector3 direction, out RaycastHit hit, float maxDistance, uint layerMask)
        {
            var intHit = new InternalRaycastHit();
            bool result = InternalCalls.Physics_RaycastLayerMask(ref origin, ref direction, out intHit, maxDistance, layerMask);

            hit.entity = InternalCalls.Entity_FindById(intHit.entityId) as Entity;
            hit.position = intHit.position;
            hit.normal = intHit.normal;
            hit.distance = intHit.distance;
            return result;
        }

        static public Entity[] OverlapBox(Vector3 origin, Vector3 halfSize, uint layerMask)
        {
            return InternalCalls.Physics_OverlapBox(ref origin, ref halfSize, layerMask);
        }

        static public Entity[] OverlapSphere(Vector3 origin, float radius, uint layerMask)
        {
            return InternalCalls.Physics_OverlapSphere(ref origin, radius, layerMask);
        }
    }
}
