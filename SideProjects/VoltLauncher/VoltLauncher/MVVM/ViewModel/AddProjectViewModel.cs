using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VoltLauncher.Core;

namespace VoltLauncher.MVVM.ViewModel
{
    class AddProjectViewModel
    {
        public RelayCommand OpenLocationCommand { get; set; }
        public RelayCommand AddCommand { get; set; }
        public RelayCommand CancelCommand { get; set; }

        public AddProjectViewModel()
        {
            OpenLocationCommand = new RelayCommand(o =>
            {
            });

            AddCommand = new RelayCommand(o => 
            { 
            
            });

            CancelCommand = new RelayCommand(o =>
            {
            });
        }
    }
}
