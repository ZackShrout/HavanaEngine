using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Input;

namespace HavanaEditor
{
    class RelayCommand<T> : ICommand
    {
        // STATE
        private readonly Action<T> execute;
        private readonly Predicate<T> canExecute;

        // PUBLIC
        public RelayCommand(Action<T> execute, Predicate<T> canExecute = null)
        {
            this.execute = execute ?? throw new ArgumentNullException(nameof(execute));
            this.canExecute = canExecute;
        }
        
        public event EventHandler CanExecuteChanged
        {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }

        public bool CanExecute(object parameter)
        {
            return canExecute?.Invoke((T)parameter) ?? true;
        }

        public void Execute(object parameter)
        {
            execute((T)parameter);
        }
    }
}
