using System;
using System.Runtime.InteropServices;

namespace Volt
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3 : IEquatable<Vector3>
    {
        public static Vector3 Zero = new Vector3(0, 0, 0);
        public static Vector3 One = new Vector3(1, 1, 1);

        public static Vector3 Forward = new Vector3(0, 0, -1);
        public static Vector3 Back = new Vector3(0, 0, 1);
        public static Vector3 Right = new Vector3(1, 0, 0);
        public static Vector3 Left = new Vector3(-1, 0, 0);
        public static Vector3 Up = new Vector3(0, 1, 0);
        public static Vector3 Down = new Vector3(0, -1, 0);

        public static Vector3 Inifinity = new Vector3(float.PositiveInfinity);

        public float x;
        public float y;
        public float z;

        public Vector3(float scalar) => x = y = z = scalar;

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public Vector3(float x, Vector2 yz)
        {
            this.x = x;
            this.y = yz.x;
            this.z = yz.y;
        }

        public Vector3(Vector2 vector)
        {
            x = vector.x;
            y = vector.y;
            z = 0.0f;
        }

        public Vector3(Vector4 vector)
        {
            x = vector.x;
            y = vector.y;
            z = vector.z;
        }

        public void Clamp(Vector3 min, Vector3 max)
        {
            x = Mathf.Clamp(x, min.x, max.x);
            y = Mathf.Clamp(y, min.y, max.y);
            z = Mathf.Clamp(z, min.z, max.z);
        }

        public void Clamp(float min, float max)
        {
            x = Mathf.Clamp(x, min, max);
            y = Mathf.Clamp(y, min, max);
            z = Mathf.Clamp(z, min, max);
        }
        public void ClampMagnitude(float maxLength)
        {
            if (sqrMagnitude > maxLength * maxLength)
            {
                float magnitude = Mathf.Sqrt(sqrMagnitude);
                float normalizedMagnitude = maxLength / magnitude;
                x *= normalizedMagnitude;
                y *= normalizedMagnitude;
                z *= normalizedMagnitude;
            }
        }

        public float Length() => (float)Math.Sqrt(x * x + y * y + z * z);

        public Vector3 Normalized()
        {
            float length = Length();
            return length > 0.0f ? New(v => v / length) : new Vector3(x, y, z);
        }

        public void Normalize()
        {
            float length = Length();

            if (length == 0f)
            {
                return;
            }

            Apply(v => v / length);
        }

        public static Vector3 DirectionFromEuler(Vector3 rotation)
        {
            float pitch = rotation.x;
            float yaw = rotation.y;
            float roll = rotation.z;

            return new Vector3(Mathf.Cos(yaw) * Mathf.Cos(pitch),
                Mathf.Sin(yaw) * Mathf.Cos(pitch),
                Mathf.Sin(pitch));
        }

        public float Distance(Vector3 other)
        {
            return (float)Math.Sqrt(Math.Pow(other.x - x, 2) +
                                    Math.Pow(other.y - y, 2) +
                                    Math.Pow(other.z - z, 2));
        }

        public static float Distance(Vector3 p1, Vector3 p2)
        {
            return (float)Math.Sqrt(Math.Pow(p2.x - p1.x, 2) +
                                    Math.Pow(p2.y - p1.y, 2) +
                                    Math.Pow(p2.z - p1.z, 2));
        }

        //Lerps from p1 to p2
        public static Vector3 Lerp(Vector3 p1, Vector3 p2, float maxDistanceDelta)
        {
            if (maxDistanceDelta < 0.0f)
                return p1;

            if (maxDistanceDelta > 1.0f)
                return p2;

            return p1 + ((p2 - p1) * maxDistanceDelta);
        }

        public const float kEpsilonNormalSqrt = 1e-15F;

        public float sqrMagnitude { get { return x * x + y * y + z * z; } }

        public static float Dot(Vector3 lhs, Vector3 rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }

        public static float Angle(Vector3 from, Vector3 to)
        {
            // sqrt(a) * sqrt(b) = sqrt(a * b) -- valid for real numbers
            float denominator = (float)Math.Sqrt(from.sqrMagnitude * to.sqrMagnitude);
            if (denominator < kEpsilonNormalSqrt)
                return 0F;

            float dot = Mathf.Clamp(Dot(from, to) / denominator, -1F, 1F);
            return ((float)Math.Acos(dot)) * Mathf.Rad2Deg;
        }

        /// <summary>
        /// Allows you to pass in a delegate that takes in
        /// a double to process the vector per axis.
        /// i.e. (Math.Cos) or a lambda (v => v * 3)
        /// </summary>
        /// <param name="func">Delegate 'double' method to act as a scalar to process X, Y and Z</param>
        public void Apply(Func<double, double> func)
        {
            x = (float)func(x);
            y = (float)func(y);
            z = (float)func(z);
        }

        /// <summary>
        /// Allows you to pass in a delegate that takes in and returns
        /// a new Vector processed per axis.
        /// i.e. (Math.Cos) or a lambda (v => v * 3)
        /// </summary>
        /// <param name="func">Delegate 'double' method to act as a scalar to process X, Y and Z</param>
        public Vector3 New(Func<double, double> func) => new Vector3((float)func(x), (float)func(y), (float)func(z));

        public static Vector3 operator *(Vector3 left, float scalar) => new Vector3(left.x * scalar, left.y * scalar, left.z * scalar);
        public static Vector3 operator *(float scalar, Vector3 right) => new Vector3(scalar * right.x, scalar * right.y, scalar * right.z);
        public static Vector3 operator *(Vector3 left, Vector3 right) => new Vector3(left.x * right.x, left.y * right.y, left.z * right.z);
        public static Vector3 operator /(Vector3 left, Vector3 right) => new Vector3(left.x / right.x, left.y / right.y, left.z / right.z);
        public static Vector3 operator /(Vector3 left, float scalar) => new Vector3(left.x / scalar, left.y / scalar, left.z / scalar);
        public static Vector3 operator /(float scalar, Vector3 right) => new Vector3(scalar / right.x, scalar / right.y, scalar / right.z);
        public static Vector3 operator +(Vector3 left, Vector3 right) => new Vector3(left.x + right.x, left.y + right.y, left.z + right.z);
        public static Vector3 operator +(Vector3 left, float right) => new Vector3(left.x + right, left.y + right, left.z + right);
        public static Vector3 operator -(Vector3 left, Vector3 right) => new Vector3(left.x - right.x, left.y - right.y, left.z - right.z);
        public static Vector3 operator -(Vector3 vector) => new Vector3(-vector.x, -vector.y, -vector.z);

        public float this[int key]
        {
            get
            {
                if (key == 0)
                {
                    return x;
                }
                else if (key == 1)
                {
                    return y;
                }

                return z;
            }

            set
            {
                if (key == 0)
                {
                    x = value;
                }
                else if (key == 1)
                {
                    y = value;
                }
                else
                {
                    z = value;
                }
            }
        }

        public static Vector3 Cross(Vector3 x, Vector3 y)
        {
            return new Vector3(
                x.y * y.z - y.y * x.z,
                x.z * y.x - y.z * x.x,
                x.x * y.y - y.x * x.y
            );
        }

        public override bool Equals(object obj) => obj is Vector3 other && Equals(other);
        public bool Equals(Vector3 right) => x == right.x && y == right.y && z == right.z;

        public override int GetHashCode() => (x, y, z).GetHashCode();

        public static bool operator ==(Vector3 left, Vector3 right) => left.Equals(right);
        public static bool operator !=(Vector3 left, Vector3 right) => !(left == right);

        public static Vector3 Cos(Vector3 vector) => vector.New(Math.Cos);
        public static Vector3 Sin(Vector3 vector) => vector.New(Math.Sin);

        public override string ToString() => $"Vector3[{x}, {y}, {z}]";

        public Vector2 XY
        {
            get => new Vector2(x, y);
            set { x = value.x; y = value.y; }
        }

        public Vector2 XZ
        {
            get => new Vector2(x, z);
            set { x = value.x; z = value.y; }
        }

        public Vector2 YZ
        {
            get => new Vector2(y, z);
            set { y = value.x; z = value.y; }
        }
    }
}
