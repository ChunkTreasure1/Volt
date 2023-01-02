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

        public Vector3(float scalar) : this()
        {
            x = scalar;
            y = scalar;
            z = scalar;
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

        public Vector3 Normalized()
        {
            return Vector3.Zero;
        }

        public Vector3 Normalize()
        {
            return this;
        }

        public Vector3 Cross(Vector3 other)
        {
            return Vector3.Zero;
        }

        public float Length()
        {
            return 0f;
        }

        public float Distance(Vector3 target)
        {
            return 0f;
        }

        public float Dot(Vector3 other)
        {
            return 0f;
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

        public static Vector3 operator *(Vector3 lhs, float rhs)
            => new Vector3(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);

        public static Vector3 operator *(float rhs, Vector3 lhs)
            => new Vector3(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);

        public static Vector3 operator /(Vector3 lhs, float rhs)
        {
            if (rhs == 0f)
            {
                throw new DivideByZeroException();
            }

            return new Vector3(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
        }

        public static bool operator ==(Vector3 lhs, Vector3 rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
        }

        public static bool operator !=(Vector3 lhs, Vector3 rhs)
        {
            return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z;
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
