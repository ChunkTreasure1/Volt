using System;

namespace Volt
{
    public static class VoltApplication
    {

        public static void LoadLevel(string aLevelAssetPath)
        {
            InternalCalls.VoltApplication_LoadLevel(aLevelAssetPath);
        }

        public static void SetResolution(uint X, uint Y)
        {
            InternalCalls.VoltApplication_SetResolution(X,Y);
        }

        public static void SetWindowMode(uint WindowMode)
        {
            InternalCalls.VoltApplication_SetWindowMode(WindowMode);
        }

        public static bool IsRuntime()
        {
            return InternalCalls.VoltApplication_IsRuntime();
        }
        
        public static void Quit()
        {
            InternalCalls.VoltApplication_Quit();
        }
    }
}
