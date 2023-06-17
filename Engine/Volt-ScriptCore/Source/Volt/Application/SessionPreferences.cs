using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Win32;

namespace Volt
{
    public static class SessionPreferences
    {
        private static Dictionary<string, object> myRegistry = new Dictionary<string, object>();

        static SessionPreferences()
        {
            string regPath = GetRegistryPath();

            RegistryKey regKey = Registry.CurrentUser.CreateSubKey(regPath);
            foreach (var v in regKey.GetValueNames())
            {
                myRegistry[v] = regKey.GetValue(v);
            }

            regKey.Close();
        }

        private static string GetRegistryPath()
        {
            bool isRuntime = VoltApplication.IsRuntime();
            string regPath = "";

            if (isRuntime)
            {
                regPath = @"Software\\" + ProjectManager.GetCompanyName() + "\\" + ProjectManager.GetProjectName();
            }
            else
            {
                regPath = @"Software\\" + "Volt\\" + ProjectManager.GetCompanyName() + "\\" + ProjectManager.GetProjectName();
            }

            return regPath;
        }

        public static void DeleteAll()
        {
            myRegistry.Clear();
        }

        public static void DeleteKey(string key)
        {
            if (myRegistry.ContainsKey(key))
            {
                myRegistry.Remove(key);
            }
        }

        public static float GetFloat(string key)
        {
            if (!myRegistry.ContainsKey(key))
            {
                return 0f;
            }

            object value = myRegistry[key];

            if (value.GetType() != typeof(float))
            {
                return 0f;
            }

            return (float)value;
        }

        public static int GetInt(string key)
        {
            if (!myRegistry.ContainsKey(key))
            {
                return 0;
            }

            object value = myRegistry[key];

            if (value.GetType() != typeof(int))
            {
                return 0;
            }

            return (int)value;
        }

        public static string GetString(string key)
        {
            if (!myRegistry.ContainsKey(key))
            {
                return "";
            }

            object value = myRegistry[key];

            if (value.GetType() != typeof(string))
            {
                return "";
            }

            return (string)value;
        }

        public static bool HasKey(string key)
        {
            return myRegistry.ContainsKey(key);
        }

        public static void Save()
        {
            string regPath = GetRegistryPath();
            RegistryKey masterKey = Registry.CurrentUser.CreateSubKey(regPath);

            foreach (KeyValuePair<string, object> entry in myRegistry)
            {
                masterKey.SetValue(entry.Key, entry.Value);
            }

            masterKey.Close();
        }

        public static void SetFloat(string key, float value)
        {
            myRegistry[key] = value;
        }

        public static void SetInt(string key, int value)
        {
            myRegistry[key] = value;
        }

        public static void SetString(string key, string value)
        {
            myRegistry[key] = value;
        }
    }
}
