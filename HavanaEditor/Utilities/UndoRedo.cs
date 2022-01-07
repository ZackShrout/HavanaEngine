using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;

namespace HavanaEditor.Utilities
{
    /// <summary>
    /// Interface to facilitate undo/redo actions in Havana Editor
    /// </summary>
    public interface IUndoRedo
    {
        // PROPERTIES
        public string Name { get; }
        
        // METHODS
        /// <summary>
        /// Handle undo command
        /// </summary>
        void Undo();
        
        /// <summary>
        /// Handle redo command
        /// </summary>
        void Redo();
    }

    public class UndoRedoAction : IUndoRedo
    {
        // PROPERTIES
        public string Name { get; }

        // ACTIONS
        private Action _undoAction;
        private Action _redoAction;

        // PUBLIC
        public UndoRedoAction(string name)
        {
            Name = name;
        }

        public UndoRedoAction(Action undo, Action redo, string name) : this(name)
        {
            Debug.Assert(undo != null && redo != null);
            _undoAction = undo;
            _redoAction = redo;
        }

        public UndoRedoAction(string property, object instance, object undoValue, object redoValue, string name)
            : this(
                  () => instance.GetType().GetProperty(property).SetValue(instance, undoValue),
                  () => instance.GetType().GetProperty(property).SetValue(instance, redoValue),
                  name) 
        { }

        /// <summary>
        /// Invokes redo action.
        /// </summary>
        public void Redo() => _redoAction();

        /// <summary>
        /// Invokes undo action.
        /// </summary>
        public void Undo() => _undoAction();
    }


    /// <summary>
    /// Undo/Redo class for Havana Editor
    /// </summary>
    public class UndoRedo
    {
        // STATE
        private bool enableAdd = true;
        private readonly ObservableCollection<IUndoRedo> redoList = new ObservableCollection<IUndoRedo>();
        private readonly ObservableCollection<IUndoRedo> undoList = new ObservableCollection<IUndoRedo>();

        // PROPERTIES
        public ReadOnlyObservableCollection<IUndoRedo> RedoList { get; }
        public ReadOnlyObservableCollection<IUndoRedo> UndoList { get; }

        // PUBLIC
        public UndoRedo()
        {
            RedoList = new ReadOnlyObservableCollection<IUndoRedo>(redoList);
            UndoList = new ReadOnlyObservableCollection<IUndoRedo>(undoList);
        }

        /// <summary>
        /// Adds an IUndoRedo object to the undo list
        /// and clears the redo list.
        /// </summary>
        public void Add(IUndoRedo command)
        {
            if (enableAdd)
            {
                undoList.Add(command);
                redoList.Clear();
            }
        }

        /// <summary>
        /// Clears both undo and redo lists.
        /// </summary>
        public void Reset()
        {
            redoList.Clear();
            undoList.Clear();
        }

        /// <summary>
        /// Calls the undo method of the last IUndoRedo in the undo list
        /// and adds it to the beginning of the redo list
        /// </summary>
        public void Undo()
        {
            if (undoList.Any())
            {
                IUndoRedo command = undoList.Last();
                undoList.RemoveAt(undoList.Count - 1);
                enableAdd = false;
                command.Undo();
                enableAdd = true;
                redoList.Insert(0, command);
            }
        }

        /// <summary>
        /// Calls the redo method of the first IUndoRedo in the redo list
        /// and adds it to the end of the undo list
        /// </summary>
        public void Redo()
        {
            if (redoList.Any())
            {
                IUndoRedo command = redoList.First();
                redoList.RemoveAt(0);
                enableAdd = false;
                command.Redo();
                enableAdd = true;
                undoList.Add(command);
            }
        }
    }
}
