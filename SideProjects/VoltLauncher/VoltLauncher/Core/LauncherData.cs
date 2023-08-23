using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

namespace VoltLauncher.Core
{
    public class LauncherData
    {
        public List<Project> Projects { get; set; } = new List<Project>();
        public string LastOpenProject { get; set; } = new string("Test");
    
        public static void Serialize(LauncherData data)
        {
            string saveLoc = new string("data/launcher.json");
            string jsonString = JsonSerializer.Serialize(data);
            File.WriteAllText(saveLoc, jsonString);
        }

        public static LauncherData? Deserialize()
        {
            if (File.Exists("data/launcher.json"))
            {
                using (StreamReader r = new StreamReader("data/launcher.json"))
                {
                    string json = r.ReadToEnd();
                    return JsonSerializer.Deserialize<LauncherData>(json);
                }
            }

            return new LauncherData();
        }
    }
}
