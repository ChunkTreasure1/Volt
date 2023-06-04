namespace Volt
{
    public static class Time
    {
        public static float deltaTime
        {
            get { return InternalCalls.Time_GetDeltaTime(); }
            private set { }
        }

        public static void SetTimeScale(float timeScale)
        {
            InternalCalls.Time_SetTimeScale(timeScale);
        }
    }
}
