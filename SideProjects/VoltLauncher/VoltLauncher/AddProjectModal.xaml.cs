using System.ComponentModel;
using System.DirectoryServices.ActiveDirectory;
using System.IO;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using Microsoft.WindowsAPICodePack.Dialogs;
using VoltLauncher.Core;

namespace VoltLauncher
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    /// 

    public partial class AddProjectModal : Window, INotifyPropertyChanged
    {
        private string myProjectName = new string("");
        private string myProjectPath = new string("");

        public string ProjectName 
        { 
            get { return myProjectName; } 
            set 
            {
                myProjectName = value; 
                OnPropertyChanged(); 
            } 
        }

        public string ProjectPath
        {
            get { return myProjectPath; }
            set
            {
                myProjectPath = value;
                OnPropertyChanged();
            }
        }

        public AddProjectModal()
        {
            InitializeComponent();
        }

        private static void CopyFilesRecursively(string sourcePath, string targetPath)
        {
            //Now Create all of the directories
            foreach (string dirPath in Directory.GetDirectories(sourcePath, "*", SearchOption.AllDirectories))
            {
                Directory.CreateDirectory(dirPath.Replace(sourcePath, targetPath));
            }

            //Copy all the files & Replaces any files with the same name
            foreach (string newPath in Directory.GetFiles(sourcePath, "*.*", SearchOption.AllDirectories))
            {
                File.Copy(newPath, newPath.Replace(sourcePath, targetPath), true);
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            string projectTemplatePath = EngineInfo.EngineDir + "\\Templates\\Project";
            string targetDir = ProjectPath + "\\" + ProjectName;

            Directory.CreateDirectory(targetDir);
            CopyFilesRecursively(projectTemplatePath, targetDir);

            // Rename project file
            {
                File.Move(targetDir + "\\Project.vtproj", targetDir + "\\" + ProjectName + ".vtproj");
            }

            Project newProj = new Project();
            newProj.Name = ProjectName;
            newProj.Path = targetDir;
        }

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            CommonOpenFileDialog dialog = new CommonOpenFileDialog();
            dialog.InitialDirectory = "C:\\Users";
            dialog.IsFolderPicker = true;
            if (dialog.ShowDialog() != CommonFileDialogResult.Ok)
            {
                return;
            }

            string fileName = dialog.FileName;
            if (fileName == "")
            {
                return;
            }

            ProjectPath= fileName;
        }

        #region INotifyPropertyChanged Members

        public event PropertyChangedEventHandler? PropertyChanged;

        /// <summary>
        /// Raises this object's PropertyChanged event.
        /// </summary>
        /// <param name="propertyName">The property that has a new value.</param>
        protected void OnPropertyChanged([CallerMemberName] string? propertyName = null)
        {
            PropertyChangedEventHandler? handler = this.PropertyChanged;
            if (handler != null)
            {
                var e = new PropertyChangedEventArgs(propertyName);
                handler(this, e);
            }
        }
        #endregion

        private void ProjectNameInput_TextChanged(object sender, TextChangedEventArgs e)
        {
            TextBox objTextBox = (TextBox)sender;
            string theText = objTextBox.Text;

            ProjectName = theText;
        }
    }
}
