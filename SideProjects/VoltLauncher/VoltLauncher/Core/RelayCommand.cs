using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace VoltLauncher.Core
{
    internal class RelayCommand : ICommand
    {
        private Action<object?>? myExecute;
        private Func<object?, bool>? myCanExecute;

        public event EventHandler? CanExecuteChanged
        {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }

        public RelayCommand(Action<object?> execute, Func<object?, bool>? canExecute = null)
        {
            myExecute = execute;
            myCanExecute = canExecute;
        }

        public bool CanExecute(object? param) => myCanExecute == null || myCanExecute(param);

        public void Execute(object? param)
        {
            if (myExecute != null)
            {
                myExecute(param);
            }
        }
    }
}
