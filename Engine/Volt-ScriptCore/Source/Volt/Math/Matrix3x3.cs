namespace Volt
{
    public class Matrix3x3
    {
        public Vector3 this[int key]
        {
            get => data[key];
            set => data[key] = value;
        }

        public Quaternion ToQuaternion()
        {
            float fourXSquaredMinus1 = data[0][0] - data[1][1] - data[2][2];
            float fourYSquaredMinus1 = data[1][1] - data[0][0] - data[2][2];
            float fourZSquaredMinus1 = data[2][2] - data[0][0] - data[1][1];
            float fourWSquaredMinus1 = data[0][0] + data[1][1] + data[2][2];

            int biggestIndex = 0;
            float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
            if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
            {
                fourBiggestSquaredMinus1 = fourXSquaredMinus1;
                biggestIndex = 1;
            }
            if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
            {
                fourBiggestSquaredMinus1 = fourYSquaredMinus1;
                biggestIndex = 2;
            }
            if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
            {
                fourBiggestSquaredMinus1 = fourZSquaredMinus1;
                biggestIndex = 3;
            }

            float biggestVal = Mathf.Sqrt(fourBiggestSquaredMinus1 + 0.25f) * 0.25f;
            float mult = 0.25f / biggestVal;

            switch (biggestIndex)
            {
                case 0:
                    return new Quaternion(biggestVal, (data[1][2] - data[2][1]) * mult, (data[2][0] - data[0][2]) * mult, (data[0][1] - data[1][0]) * mult);

                case 1:
                    return new Quaternion((data[1][2] - data[2][1]) * mult, biggestVal, (data[0][1] + data[1][0]) * mult, (data[2][0] + data[0][2]) * mult);

                case 2:
                    return new Quaternion((data[2][0] - data[0][2]) * mult, (data[0][1] + data[1][0]) * mult, biggestVal, (data[1][2] + data[2][1]) * mult);

                case 3:
                    return new Quaternion((data[0][1] - data[1][0]) * mult, (data[2][0] + data[0][2]) * mult, (data[1][2] + data[2][1]) * mult, biggestVal);

                default:
                    return new Quaternion(1, 0, 0, 0);
            }
        }

        public Vector3[] data = new Vector3[3];
    }
}
