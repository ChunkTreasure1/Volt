using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VoltLauncher.Core;
using System.Text.Json;
using YamlDotNet.Serialization;

namespace VoltLauncher.MVVM.ViewModel
{
    public struct ProjectContainer
    {
        public struct ProjectInfo
        {
            public string Name { get; set; }
            public string AssetRegistry { get; set; }
            public string AssetsPath { get; set; }
            public string EnginePath { get; set; }
            public string ShadersPath { get; set; }
        };

        public ProjectInfo Project { get; set; }

    }

    internal class ProjectsViewModel
    {
        public RelayCommand OpenProjectCommand { get; set; }
        public RelayCommand AddProjectCommand { get; set; }
        public RelayCommand LocateProjectCommand { get; set; }

        public ObservableCollection<Project> AllProjects { get; set; } = new ObservableCollection<Project>();

        public ProjectsViewModel()
        {
            MainWindow.LauncherInfo = LauncherData.Deserialize();

            if (MainWindow.LauncherInfo != null)
            {
                for (int i = 0; i < MainWindow.LauncherInfo.Projects.Count; i++)
                {
                    AllProjects.Add(MainWindow.LauncherInfo.Projects.ElementAt(i));
                }
            }

            OpenProjectCommand = new RelayCommand(o =>
            {
                Project? proj = o as Project;
                if (proj == null)
                {
                    return;
                }

                if (!File.Exists(proj.Path))
                {
                    return;
                }

                string? engineDir = EngineInfo.EngineDir;
                if (engineDir == null)
                {
                    return;
                }

                if (!Directory.Exists(engineDir))
                {
                    return;
                }

                string sandboxPath = engineDir + "\\Sandbox.exe";

                ProcessStartInfo startInfo = new ProcessStartInfo();
                startInfo.FileName = sandboxPath;
                startInfo.Arguments = proj.Path;

                Process.Start(startInfo);
            });

            AddProjectCommand = new RelayCommand(o => 
            {
                AddProjectModal modalWindow = new AddProjectModal();
                modalWindow.Owner = MainWindow.Instance;
                modalWindow.ShowDialog();
            });

            LocateProjectCommand = new RelayCommand(o =>
            {
                OpenFileDialog openFileDialog = new OpenFileDialog();
                if (openFileDialog.ShowDialog() == false)
                {
                    return;
                }

                string fileName = openFileDialog.FileName;
                string ext = Path.GetExtension(fileName);

                if (ext != ".vtproj")
                {
                    return;
                }

                var deserializer = new DeserializerBuilder().Build();

                Project newProj = new Project();
                newProj.Path = fileName;

                using (StreamReader r = new StreamReader(fileName))
                {
                    string projFile = r.ReadToEnd();
                    var root = deserializer.Deserialize<ProjectContainer>(projFile);
                    newProj.Name = root.Project.Name;
                }

                MainWindow.LauncherInfo?.Projects.Add(newProj);
                AllProjects.Add(newProj);
            });
        }
    }
}
