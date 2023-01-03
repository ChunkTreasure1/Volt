using System;

namespace Volt
{
    public struct Vector3
    {
        public float x;
        public float y;
        public float z;

        public static Vector3 Zero => new Vector3(0f);
        public static Vector3 One => new Vector3(1f);
        public static Vector3 Up => new Vector3(0f, 1f, 0f);
        public static Vector3 Right => new Vector3(1f, 0f, 0f);
        public static Vector3 Forward => new Vector3(0f, 0f, 1f);

        public Vector3 normalized
        {
            get
            {
                return this * (1f / Mathf.Sqrt(Dot(this, this)));
            }

            private set { }
        }
        
        public float magnitude
        {
            get
            {
                return Mathf.Sqrt(Dot(this, this));
            }

            private set { }
        }
        public Vector3(float scalar) : this()
        {
            x = scalar;
            y = scalar;
            z = scalar;
        }

        public Vector3(Vector3 v)
        {
            x = v.x;
            y = v.y;
            z = v.z;
        }

        public Vector3(float aX, float aY, float aZ) : this()
        {
            x = aX;
            y = aY;
            z = aZ;
        }

        public override string ToString()
        {
            return base.ToString() + ": " + $"X: {x}, Y: {y}, Z: {z}";
        }

        public static Vector3 Normalize(Vector3 v)
        {
            return v.normalized;
        }

        public static Vector3 Cross(Vector3 lhs, Vector3 rhs)
        {
            return new Vector3(
                lhs.y * rhs.z - rhs.y * lhs.z,
                lhs.z * rhs.x - rhs.z * lhs.x,
                lhs.x * rhs.y - rhs.x * lhs.y);
        }

        public static float Distance(Vector3 lhs, Vector3 rhs)
        {
            return (lhs - rhs).Length();
        }

        public float Length()
        {
            return Mathf.Sqrt(Dot(this, this));
        }

        public static float Dot(Vector3 lhs, Vector3 rhs)
        {
            Vector3 result = new Vector3(lhs * rhs);
            return result.x + result.y + result.z;
        }

        public static Vector3 Lerp(Vector3 lhs, Vector3 rhs, float t)
        {
            return (1f - t) * lhs + t * rhs;
        }

        public static Vector3 Max(Vector3 lhs, Vector3 rhs)
        {
            return lhs < rhs ? rhs : lhs;
        }

        public static Vector3 Min(Vector3 lhs, Vector3 rhs)
        {
            return lhs < rhs ? lhs : rhs;
        }

        public static Vector3 operator -(Vector3 v)
            => new Vector3(-v.x, -v.y, -v.z);

        public static Vector3 operator +(Vector3 v)
            => v;

        public static Vector3 operator -(Vector3 lhs, Vector3 rhs)
            => new Vector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);

        public static Vector3 operator +(Vector3 lhs, Vector3 rhs)
            => new Vector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);

        public static Vector3 operator *(Vector3 lhs, Vector3 rhs)
            => new Vector3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);

        public static Vector3 operator /(Vector3 lhs, Vector3 rhs)
        {
            if (rhs.x == 0f)
            {
                throw new DivideByZeroException();
            }

            if (rhs.y == 0f)
            {
                throw new DivideByZeroException();
            }

            if (rhs.z == 0f)
            {
                throw new DivideByZeroException();
            }

            return new Vector3(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
        }

        public static Vector3 operator /(Vector3 lhs, float rhs)
        {
            if (rhs == 0f)
            {
                throw new DivideByZeroException();
            }

            return new Vector3(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
        }

        public static Vector3 operator *(Vector3 lhs, float rhs)
            => new Vector3(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);

        public static Vector3 operator *(float rhs, Vector3 lhs)
            => new Vector3(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);

        public static bool operator ==(Vector3 lhs, Vector3 rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
        }

        public static bool operator !=(Vector3 lhs, Vector3 rhs)
        {
            return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z;
        }

        public static bool operator< (Vector3 lhs, Vector3 rhs)
        {
            return lhs.x < rhs.x && lhs.y < rhs.y && lhs.z < rhs.z;
        }

        public static bool operator >(Vector3 lhs, Vector3 rhs)
        {
            return lhs.x > rhs.x && lhs.y > rhs.y && lhs.z > rhs.z;
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
