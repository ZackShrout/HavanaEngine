using HavanaEditor.Components;
using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Text;
using System.Windows.Input;

namespace HavanaEditor.GameProject
{
    /// <summary>
    /// Describes any scene that may exist within a Havana Project.
    /// </summary>
    [DataContract]
    class Scene : ViewModelBase
    {
        // STATE
        private string name;
        private bool isActive;
        [DataMember(Name = nameof(GameEntities))]
        private readonly ObservableCollection<GameEntity> gameEntities = new ObservableCollection<GameEntity>();
        
        // PROPERTIES
        [DataMember]
        public string Name
        {
            get => name;
            set
            {
                if (name != value)
                {
                    name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }
        [DataMember]
        public Project Project { get; private set; }
        [DataMember]
        public bool IsActive
        {
            get => isActive;
            set
            {
                if (isActive != value)
                {
                    isActive = value;
                    OnPropertyChanged(nameof(IsActive));
                }
            }
        }
        public ReadOnlyObservableCollection<GameEntity> GameEntities { get; private set; }
        public ICommand AddGameEntityCommand { get; private set; }
        public ICommand RemoveGameEntityCommand { get; private set; }

        // PUBLIC
        public Scene(Project project, string name)
        {
            Debug.Assert(project != null);
            Project = project;
            Name = name;
            OnDeserialized(new StreamingContext());
        }

        // PRIVATE
        [OnDeserialized]
        private void OnDeserialized(StreamingContext context)
        {
            if (gameEntities != null)
            {
                GameEntities = new ReadOnlyObservableCollection<GameEntity>(gameEntities);
                OnPropertyChanged(nameof(GameEntities));
            }

            AddGameEntityCommand = new RelayCommand<GameEntity>(x =>
            {
                AddGameEntity(x);
                int entityIndex = gameEntities.Count - 1;
                Project.UndoRedo.Add(new UndoRedoAction(
                    () => RemoveGameEntity(x),
                    () => gameEntities.Insert(entityIndex, x),
                    $"Add {x.Name} to {Name}"));
            });

            RemoveGameEntityCommand = new RelayCommand<GameEntity>(x => {
                int entityIndex = gameEntities.IndexOf(x);
                RemoveGameEntity(x);
                Project.UndoRedo.Add(new UndoRedoAction(
                    () => gameEntities.Insert(entityIndex, x),
                    () => RemoveGameEntity(x),
                    $"Remove {x.Name} from {Name}"));
            });
        }

        private void AddGameEntity(GameEntity entity)
        {
            Debug.Assert(!gameEntities.Contains(entity));
            gameEntities.Add(entity);
        }

        private void RemoveGameEntity(GameEntity entity)
        {
            Debug.Assert(gameEntities.Contains(entity));
            gameEntities.Remove(entity);
        }
    }
}
