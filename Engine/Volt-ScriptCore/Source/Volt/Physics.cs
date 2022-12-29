namespace Volt
{
    public struct RaycastHit
    {
        Entity entity;
        Vector3 position;
        Vector3 normal;
        float distance;
    }

    public class Physics
    {
        static bool Raycast(Vector3 origin, Vector3 direction, out RaycastHit hit, float maxDistance)
        {
            return InternalCalls.Physics_Raycast(ref origin, ref direction, out hit, maxDistance);
        }

        static bool Raycast(Vector3 origin, Vector3 direction, out RaycastHit hit, float maxDistance, uint layerMask)
        {
            return InternalCalls.Physics_RaycastLayerMask(ref origin, ref direction, out hit, maxDistance, layerMask);
        }
    }
}
