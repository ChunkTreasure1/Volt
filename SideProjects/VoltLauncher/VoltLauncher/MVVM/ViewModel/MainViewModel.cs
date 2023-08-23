using System;
using VoltLauncher.Core;

namespace VoltLauncher.MVVM.ViewModel
{
    internal class MainViewModel : ObservableObject
    {
        public RelayCommand ProjectsViewCommand { get; set; }
        public RelayCommand EngineViewCommand { get; set; }

        public ProjectsViewModel ProjectsVM { get; set; }
        public EngineViewModel EngineVM { get; set; }

        private object? myCurrentView;
        public object? CurrentView
        {
            get { return myCurrentView; }
            set 
            { 
                myCurrentView = value; 
                OnPropertyChanged();
            }
        }

        public MainViewModel()
        {
            ProjectsVM = new ProjectsViewModel();
            EngineVM = new EngineViewModel();
            CurrentView = ProjectsVM;

            ProjectsViewCommand = new RelayCommand(o => 
            {
                CurrentView = ProjectsVM;
            });

            EngineViewCommand = new RelayCommand(o =>
            {
                CurrentView = EngineVM;
            });
        }
    }
}
