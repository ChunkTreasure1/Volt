using System;
using System.Runtime.InteropServices;
using System.Security;

namespace Volt
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Quaternion : IEquatable<Quaternion>
    {
        public float x;
        public float y;
        public float z;
        public float w;

        public static Quaternion Identity = new Quaternion(1, 0, 0, 0);

        public Quaternion(float w, float y, float z, float x)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public Quaternion(Vector3 xyz, float w)
        {
            this.x = xyz.x;
            this.y = xyz.y;
            this.z = xyz.z;
            this.w = w;
        }

        public Quaternion(Vector3 euler)
        {
            Vector3 c = Vector3.Cos(euler * 0.5f);
            Vector3 s = Vector3.Sin(euler * 0.5f);

            this.w = c.x * c.y * c.z + s.x * s.y * s.z;
            this.x = s.x * c.y * c.z - c.x * s.y * s.z;
            this.y = c.x * s.y * c.z + s.x * c.y * s.z;
            this.z = c.x * c.y * s.z - s.x * s.y * c.z;
        }

        public static Quaternion Euler(float x, float y, float z)
        {
            return new Quaternion(new Vector3(x, y, z));
        }

        public static Quaternion Euler(Vector3 xyz)
        {
            return new Quaternion(xyz);
        }

        public static Vector3 operator *(Quaternion q, Vector3 v)
        {
            Vector3 qv = new Vector3(q.x, q.y, q.z);
            Vector3 uv = Vector3.Cross(qv, v);
            Vector3 uuv = Vector3.Cross(qv, uv);
            return v + ((uv * q.w) + uuv) * 2.0f;
        }

        public static Quaternion operator*(Quaternion lhs, Quaternion rhs)
        {
            Quaternion newQuat = new Quaternion();
            newQuat.w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
            newQuat.x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
            newQuat.y = lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z;
            newQuat.z = lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x;

            return newQuat;
        }

        public static Quaternion operator /(Quaternion lhs, float rhs)
        {
            Quaternion newQuat = new Quaternion();
            newQuat.w = lhs.w / rhs;
            newQuat.x = lhs.x / rhs;
            newQuat.y = lhs.y / rhs;
            newQuat.z = lhs.z / rhs;

            return newQuat;
        }

        public override int GetHashCode() => (w, x, y, z).GetHashCode();

        public override bool Equals(object obj) => obj is Quaternion other && Equals(other);

        public bool Equals(Quaternion right) => w == right.w && x == right.x && y == right.y && z == right.z;

        public static bool operator ==(Quaternion left, Quaternion right) => left.Equals(right);
        public static bool operator !=(Quaternion left, Quaternion right) => !(left == right);

        [SecuritySafeCritical]
        public unsafe byte[] GetBytes()
        {
            byte[] bytes = new byte[sizeof(Quaternion)];
            fixed (byte* ptr = bytes)
            {
                *(Quaternion*)ptr = this;
            }

            return bytes;
        }
        public float Length
        {
            get
            {
                return (float)System.Math.Sqrt(LengthSquared);
            }
        }

        public float LengthSquared
        {
            get
            {
                return x * x + y * y + z * z + w * w;
            }
        }

        public Quaternion conjugate
        {
            get => new Quaternion(w, -x, -y, -z);
            private set
            { }
        }

        public Quaternion inverse
        {
            get
            {
                return conjugate / Dot(this, this);
            }
        }

        public float Roll()
        {
            return Mathf.Atan2(2f * (x * y + w * z), w * w + x * x - y * y - z * z);
        }

        public float Pitch()
        {
            float pY = 2f * (y * z + w * x);
            float pX = w * w - x * x - y * y + z * z;

            if (x <= Mathf.Epsilon && y <= Mathf.Epsilon)
            { 
                return 2f * Mathf.Atan2(x, w);
            }

            return Mathf.Atan2(pY, pX);
        }

        public float Yaw()
        {
            return Mathf.Asin(Mathf.Clamp(-2f * (x * z - w * y), -1f, 1f));
        }

        public Vector3 EulerAngles()
        {   
            return new Vector3(Pitch(), Yaw(), Roll());
        }

        public Vector3 eulerAngles
        {
            get => new Vector3(Pitch(), Yaw(), Roll());
            set
            {
                this = new Quaternion(value);
            }
        }

        public Vector3 XYZ
        {
            get => new Vector3(x, y, z);
            set
            {
                x = value.x;
                y = value.y;
                z = value.z;
            }
        }

        public static float Dot(Quaternion lhs, Quaternion rhs)
        {
            Vector4 tmp = new Vector4(lhs.w * rhs.w, lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
            return (tmp.x + tmp.y) + (tmp.z + tmp.w);
        }

        public static float Angle(Quaternion a, Quaternion b)
        {
            float dot = Quaternion.Dot(a, b);

            // Clamp the dot product to ensure it remains within the valid range of -1 to 1
            dot = Mathf.Clamp(dot, -1f, 1f);

            // Calculate the angle between the two quaternions using the arccosine of the dot product
            float angle = Mathf.Acos(dot) * Mathf.Rad2Deg;

            return angle;
        }

        public static Quaternion RotateTowards(Quaternion currentRotation, Quaternion targetRotation, float maxDegreesDelta)
        {
            // Find the angle between the current rotation and the target rotation
            float angle = Quaternion.Angle(currentRotation, targetRotation);

            // If the angle is already within the threshold, return the target rotation
            if (angle <= maxDegreesDelta)
            {
                return targetRotation;
            }

            // Calculate the fraction of the angle to rotate based on maxDegreesDelta
            float t = maxDegreesDelta / angle;

            // Spherically interpolate (slerp) between the current and target rotation
            return Slerp(currentRotation, targetRotation, t);
        }

        public static Quaternion FromToRotation(Vector3 aFrom, Vector3 aTo)
        {
            Vector3 axis = Vector3.Cross(aFrom, aTo);
            float angle = Vector3.Angle(aFrom, aTo);
            return Quaternion.AngleAxis(angle, axis.Normalized());
        }

        public static Quaternion AngleAxis(float aAngle, Vector3 aAxis)
        {
            aAxis.Normalize();
            float rad = aAngle * Mathf.Deg2Rad * 0.5f;
            aAxis *= Mathf.Sin(rad);
            return new Quaternion(aAxis.x, aAxis.y, aAxis.z, Mathf.Cos(rad));
        }

        public static Quaternion QuaternionLookRotation(Vector3 direction, Vector3 up)
        {
            direction.Normalize();
            up.Normalize();

            Vector3 vector = direction.Normalized();
            Vector3 vector2 = Vector3.Cross(-up, vector).Normalized();
            Vector3 vector3 = Vector3.Cross(vector, vector2);
            var m00 = vector2.x;
            var m01 = vector2.y;
            var m02 = vector2.z;
            var m10 = vector3.x;
            var m11 = vector3.y;
            var m12 = vector3.z;
            var m20 = vector.x;
            var m21 = vector.y;
            var m22 = vector.z;

            float num8 = (m00 + m11) + m22;
            var quaternion = new Quaternion();
            if (num8 > 0f)
            {
                var num = (float)Math.Sqrt(num8 + 1f);
                quaternion.w = num * 0.5f;
                num = 0.5f / num;
                quaternion.x = (m12 - m21) * num;
                quaternion.y = (m20 - m02) * num;
                quaternion.z = (m01 - m10) * num;
                return quaternion;
            }
            if ((m00 >= m11) && (m00 >= m22))
            {
                var num7 = (float)Math.Sqrt(((1f + m00) - m11) - m22);
                var num4 = 0.5f / num7;
                quaternion.x = 0.5f * num7;
                quaternion.y = (m01 + m10) * num4;
                quaternion.z = (m02 + m20) * num4;
                quaternion.w = (m12 - m21) * num4;
                return quaternion;
            }
            if (m11 > m22)
            {
                var num6 = (float)Math.Sqrt(((1f + m11) - m00) - m22);
                var num3 = 0.5f / num6;
                quaternion.x = (m10 + m01) * num3;
                quaternion.y = 0.5f * num6;
                quaternion.z = (m21 + m12) * num3;
                quaternion.w = (m20 - m02) * num3;
                return quaternion;
            }
            var num5 = (float)Math.Sqrt(((1f + m22) - m00) - m11);
            var num2 = 0.5f / num5;
            quaternion.x = (m20 + m02) * num2;
            quaternion.y = (m21 + m12) * num2;
            quaternion.z = 0.5f * num5;
            quaternion.w = (m01 - m10) * num2;
            return quaternion;
        }

        public void Normalize()
        {
            float scale = 1.0f / this.Length;
            XYZ *= scale;
            w *= scale;
        }

        public static Quaternion Slerp(Quaternion a, Quaternion b, float t)
        {
            if (t > 1) t = 1;
            if (t < 0) t = 0;
            return SlerpUnclamped(a, b, t);
        }

        public static Quaternion SlerpUnclamped(Quaternion a, Quaternion b, float t)
        {
            // if either input is zero, return the other.
            if (a.LengthSquared == 0.0f)
            {
                if (b.LengthSquared == 0.0f)
                {
                    return new Quaternion(0, 0, 0, 1);
                }
                return b;
            }
            else if (b.LengthSquared == 0.0f)
            {
                return a;
            }


            float cosHalfAngle = a.w * b.w + Vector3.Dot(a.XYZ, b.XYZ);

            if (cosHalfAngle >= 1.0f || cosHalfAngle <= -1.0f)
            {
                // angle = 0.0f, so just return one input.
                return a;
            }
            else if (cosHalfAngle < 0.0f)
            {
                b.XYZ = -b.XYZ;
                b.w = -b.w;
                cosHalfAngle = -cosHalfAngle;
            }

            float blendA;
            float blendB;
            if (cosHalfAngle < 0.99f)
            {
                // do proper slerp for big angles
                float halfAngle = (float)System.Math.Acos(cosHalfAngle);
                float sinHalfAngle = (float)System.Math.Sin(halfAngle);
                float oneOverSinHalfAngle = 1.0f / sinHalfAngle;
                blendA = (float)System.Math.Sin(halfAngle * (1.0f - t)) * oneOverSinHalfAngle;
                blendB = (float)System.Math.Sin(halfAngle * t) * oneOverSinHalfAngle;
            }
            else
            {
                // do lerp if angle is really small.
                blendA = 1.0f - t;
                blendB = t;
            }

            Quaternion result = new Quaternion(blendA * a.XYZ + blendB * b.XYZ, blendA * a.w + blendB * b.w);
            if (result.LengthSquared > 0.0f)
            {
                result.Normalize();
                return result;
            }

            return new Quaternion(0, 0, 0, 1);
        }

    }
}
