using System;

namespace Volt
{
    public struct Quaternion
    {
        public float x;
        public float y;
        public float z;
        public float w;
        public static Quaternion Identity => new Quaternion(1f, 0f, 0f, 0f);

        public Quaternion(Vector3 eulerDegreeAngles)
        {
            Vector3 radianAngles = Mathf.Radians(eulerDegreeAngles);
            Vector3 c = Mathf.Cos(radianAngles * 0.5f);
            Vector3 s = Mathf.Sin(radianAngles * 0.5f);

            w = c.x * c.y * c.z + s.x * s.y * s.z;
            x = s.x * c.y * c.z - c.x * s.y * s.z;
            y = c.x * s.y * c.z + s.x * c.y * s.z;
            z = c.x * c.y * s.z - s.x * s.y * c.z;
        }

        public Quaternion(float eulerDegreesX, float eulerDegreesY, float eulerDegreesZ) : this()
        {
            Vector3 radianAngles = Mathf.Radians(new Vector3(eulerDegreesX, eulerDegreesY, eulerDegreesZ));
            Vector3 c = Mathf.Cos(radianAngles * 0.5f);
            Vector3 s = Mathf.Sin(radianAngles * 0.5f);

            w = c.x * c.y * c.z + s.x * s.y * s.z;
            x = s.x * c.y * c.z - c.x * s.y * s.z;
            y = c.x * s.y * c.z + s.x * c.y * s.z;
            z = c.x * c.y * s.z - s.x * s.y * c.z;
        }

        public Quaternion(float aW, float aX, float aY, float aZ) : this()
        {
            x = aX;
            y = aY;
            z = aZ;
            w = aW;
        }

        public Quaternion(Quaternion q)
        {
            x = q.x;
            y = q.y;
            z = q.z;
            w = q.w;
        }

        public Quaternion(float s, Vector3 v)
        {
            x = v.x;
            y = v.y;
            z = v.z;

            w = s;
        }

        public Vector3 eulerAngles
        {
            get
            {
                return new Vector3(Pitch(), Yaw(), Roll());
            }

            set
            {
                this = new Quaternion(value);
            }
        }

        public Quaternion normalized
        {
            get
            {
                return this * (1f / Mathf.Sqrt(Dot(this, this)));
            }

            private set { }
        }

        public static Quaternion AngleAxis(float rotDegrees, Vector3 direction)
        {
            float a = Mathf.Radians(rotDegrees);
            float s = Mathf.Sin(a * 0.5f);

            return new Quaternion(Mathf.Cos(a * 0.5f), direction * s);
        }

        public static float Dot(Quaternion lhs, Quaternion rhs)
        {
            return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
        }

        public static Quaternion Euler(float xDegrees, float yDegrees, float zDegrees)
        {
            return new Quaternion(xDegrees, yDegrees, zDegrees);
        }

        public static Quaternion FromToRotation(Vector3 lhs, Vector3 rhs)
        {
            float d = Vector3.Dot(lhs, rhs);

            if (d > -0.999999f)
            {
                Vector3 c = Vector3.Cross(lhs, rhs);
                float s = Mathf.Sqrt((1f + d) * 2f);
                float invs = 1f / s;

                return new Quaternion(s * 0.5f, c * invs);
            }
            else
            {
                Vector3 axis = Vector3.Cross(Vector3.Right, lhs);
                if (axis.magnitude < 0.000001f)
                    axis = Vector3.Cross(Vector3.Up, lhs);

                return AngleAxis(180f, axis);
            }
        }

        public static Quaternion Inverse(Quaternion q)
        {
            float lengthSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
            if (lengthSq > 0.0f)
            {
                float i = 1.0f / lengthSq;
                return new Quaternion(q.w * i, -q.x * i, -q.y * i, -q.z * i);
            }
            return q;
        }

        public static Quaternion Lerp(Quaternion a, Quaternion rhs, float t)
        {
            float dot = Dot(a, rhs);

            if (dot < 0.0f)
            {
                rhs = new Quaternion(-rhs.x, -rhs.y, -rhs.z, -rhs.w);
                dot = -dot;
            }

            if (dot > 0.999999f)
            {
                Quaternion result = new Quaternion(a.x + (rhs.x - a.x) * t, a.y + (rhs.y - a.y) * t, a.z + (rhs.z - a.z) * t, a.w + (rhs.w - a.w) * t);
                return result.normalized;
            }

            float theta0 = Mathf.Acos(dot);
            float theta = theta0 * t;

            Quaternion q = rhs - a * dot;

            return a * Mathf.Cos(theta) + q.normalized * Mathf.Sin(theta);
        }

        public static Quaternion LookRotation(Vector3 forward, Vector3 up)
        {
            Vector3 right = Vector3.Cross(up, forward);
            up = Vector3.Cross(forward, right);

            float m00 = right.x;
            float m01 = right.y;
            float m02 = right.z;
            float m10 = up.x;
            float m11 = up.y;
            float m12 = up.z;
            float m20 = forward.x;
            float m21 = forward.y;
            float m22 = forward.z;

            float num8 = (m00 + m11) + m22;
            Quaternion quaternion = new Quaternion();
            if (num8 > 0f)
            {
                float num = Mathf.Sqrt(num8 + 1f);
                quaternion.w = num * 0.5f;
                num = 0.5f / num;
                quaternion.x = (m12 - m21) * num;
                quaternion.y = (m20 - m02) * num;
                quaternion.z = (m01 - m10) * num;
                return quaternion;
            }
            if ((m00 >= m11) && (m00 >= m22))
            {
                float num7 = Mathf.Sqrt(((1f + m00) - m11) - m22);
                float num4 = 0.5f / num7;
                quaternion.x = 0.5f * num7;
                quaternion.y = (m01 + m10) * num4;
                quaternion.z = (m02 + m20) * num4;
                quaternion.w = (m12 - m21) * num4;
                return quaternion;
            }
            if (m11 > m22)
            {
                float num6 = Mathf.Sqrt(((1f + m11) - m00) - m22);
                float num3 = 0.5f / num6;
                quaternion.x = (m10 + m01) * num3;
                quaternion.y = 0.5f * num6;
                quaternion.z = (m21 + m12) * num3;
                quaternion.w = (m20 - m02) * num3;
                return quaternion;
            }
            float num5 = Mathf.Sqrt(((1f + m22) - m00) - m11);
            float num2 = 0.5f / num5;
            quaternion.x = (m20 + m02) * num2;
            quaternion.y = (m21 + m12) * num2;
            quaternion.z = 0.5f * num5;
            quaternion.w = (m01 - m10) * num2;
            return quaternion;
        }

        public static Quaternion Normalize(Quaternion normalize)
        {
            float length = Mathf.Sqrt(normalize.x * normalize.x + normalize.y * normalize.y + normalize.z * normalize.z + normalize.w * normalize.w);
            if (length > 0.00001f)
            {
                return new Quaternion(normalize.x / length, normalize.y / length, normalize.z / length, normalize.w / length);
            }
            else
            {
                return new Quaternion(0, 0, 0, 0);
            }
        }

        public static Quaternion Slerp(Quaternion a, Quaternion b, float t)
        {
            float dot = Dot(a, b);

            if (dot < 0.0f)
            {
                b = new Quaternion(-b.x, -b.y, -b.z, -b.w);
                dot = -dot;
            }

            if (dot > 0.999999f)
            {
                Quaternion result = new Quaternion(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
                return result.normalized;
            }

            float theta0 = Mathf.Acos(dot);
            float theta = theta0 * t;

            Quaternion q = b - a * dot;

            return a * Mathf.Cos(theta) + q.normalized * Mathf.Sin(theta);
        }

        public static Quaternion operator -(Quaternion a)
        {
            return new Quaternion(-a.x, -a.y, -a.z, -a.w);
        }

        public static Quaternion operator+(Quaternion a)
        {
            return new Quaternion(a.x, a.y, a.z, a.w);
        }

        public static Quaternion operator -(Quaternion a, Quaternion b)
        {
            return new Quaternion(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
        }

        public static Quaternion operator +(Quaternion a, Quaternion b)
        {
            return new Quaternion(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        }

        public static Quaternion operator *(Quaternion a, Quaternion b)
        {
            return new Quaternion(a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y, a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z, a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x, a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z);
        }

        public static Quaternion operator *(Quaternion a, float b)
        {
            return new Quaternion(a.x * b, a.y * b, a.z * b, a.w * b);
        }

        public static Quaternion operator *(float a, Quaternion b)
        {
            return new Quaternion(b.x * a, b.y * a, b.z * a, b.w * a);
        }

        public static bool operator ==(Quaternion a, Quaternion b)
        {
            return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
        }

        public static bool operator !=(Quaternion a, Quaternion b)
        {
            return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
        }

        private float Pitch()
        {
            float pY = 2f * (y * z + w * x);
            float pX = w * w - x * x - y * y + z * z;

            if (pX == 0f && pY == 0f)
            {
                return 2f * (float)Math.Atan2(x, w);
            }

            return (float)Math.Atan2(pY, pX);
        }

        private float Yaw()
        {
            return (float)Math.Asin(Mathf.Clamp(-2f * (x * z - w * y), -1f, 1f));
        }

        private float Roll()
        {
            return (float)Math.Atan2(2f * x * y + w * z, w * w + x * x - y * y - z * z);
        }

        public override bool Equals(object obj)
        {
            return base.Equals(obj);
        }
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }
    }
}
