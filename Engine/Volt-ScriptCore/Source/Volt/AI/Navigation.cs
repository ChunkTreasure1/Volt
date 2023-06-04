namespace Volt
{
    namespace AI
    {
        public class Navigation
        {
            public static Vector3[] FindPath(Vector3 start, Vector3 end, Vector3 polygonSearchDistance)
            {
                return InternalCalls.Navigation_FindPath(ref start, ref end, ref polygonSearchDistance);
            }
        }
    }
}
