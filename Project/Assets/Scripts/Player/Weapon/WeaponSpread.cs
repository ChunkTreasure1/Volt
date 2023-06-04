using Volt;

namespace Project
{
    public class WeaponSpread
    {
        public static Vector3 GetRandomSpreadDirection(Vector3 aimDirection, float maxSpreadAngle)
        {
            float rad = Mathf.Radians(maxSpreadAngle);

            Quaternion randomRotation = Quaternion.Euler(
                Random.Range(-rad, rad),
                Random.Range(-rad, rad),
                Random.Range(-rad, rad)
            );

            Vector3 spreadDirection = randomRotation * aimDirection;
            spreadDirection.Normalize();

            return spreadDirection;
        }
    }
}
