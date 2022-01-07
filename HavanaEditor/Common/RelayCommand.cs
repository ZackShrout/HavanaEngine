using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Input;

namespace HavanaEditor
{
    class RelayCommand<T> : ICommand
    {
        // STATE
        private readonly Action<T> _execute;
        private readonly Predicate<T> _canExecute;

        // PUBLIC
        public RelayCommand(Action<T> execute, Predicate<T> canExecute = null)
        {
            this._execute = execute ?? throw new ArgumentNullException(nameof(execute));
            this._canExecute = canExecute;
        }
        
        public event EventHandler CanExecuteChanged
        {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }

        public bool CanExecute(object parameter)
        {
            return _canExecute?.Invoke((T)parameter) ?? true;
        }

        public void Execute(object parameter)
        {
            _execute((T)parameter);
        }
    }
}
