using HavanaEditor.GameProject;
using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Windows.Input;

namespace HavanaEditor.Components
{
    [DataContract]
    [KnownType(typeof(Transform))]
    class GameEntity : ViewModelBase
    {
        // STATE
        private bool isEnabled = true;
        private string name;
        [DataMember(Name = nameof(Components))]
        private readonly ObservableCollection<Component> components = new ObservableCollection<Component>();

        // PROPERTIES
        [DataMember]
        public bool IsEnabled
        {
            get => isEnabled;
            set
            {
                if (isEnabled != value)
                {
                    isEnabled = value;
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }
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
        public Scene ParentScene { get; private set; }
        public ReadOnlyObservableCollection<Component> Components { get; private set; }

        // PUBLIC
        public GameEntity(Scene scene)
        {
            Debug.Assert(scene != null);
            ParentScene = scene;
            components.Add(new Transform(this));
            OnDeserialized(new StreamingContext());
        }

        // PRIVATE
        [OnDeserialized]
        private void OnDeserialized(StreamingContext context)
        {
            if (components != null)
            {
                Components = new ReadOnlyObservableCollection<Component>(components);
                OnPropertyChanged(nameof(Components));
            }
        }
    }

    abstract class MSEntity : ViewModelBase
    {
        // STATE
        private bool enableUpdates = true;
        private bool? isEnabled = true;
        private string name;
        private readonly ObservableCollection<IMSComponent> components = new ObservableCollection<IMSComponent>();

        // PROPERTIES
        public bool? IsEnabled
        {
            get => isEnabled;
            set
            {
                if (isEnabled != value)
                {
                    isEnabled = value;
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }
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
        public ReadOnlyObservableCollection<IMSComponent> Components { get; private set; }
        public List<GameEntity> SelectedEntities { get; }

        // PUBLIC
        public MSEntity(List<GameEntity> entities)
        {
            Debug.Assert(entities?.Any() == true);
            Components = new ReadOnlyObservableCollection<IMSComponent>(components);
            SelectedEntities = entities;
            PropertyChanged += (s, e) => { if (enableUpdates) UpdateGameEntities(e.PropertyName); };
        }

        /// <summary>
        /// Updates all entities in the selected entities list.
        /// </summary>
        public void Refresh()
        {
            enableUpdates = false;
            UpdateMSGameEntities();
            enableUpdates = true;
        }
        
        /// <summary>
        /// Goes through a specified property in each selected entity
        /// and checks whether or not they are the same.
        /// </summary>
        /// <param name="entities">List of selected entities to check.</param>
        /// <param name="getProperty">Property in each selected entity to compare.</param>
        /// <returns>True, false, or null.</returns>
        public static float? GetMixedValue(List<GameEntity> entities, Func<GameEntity, float> getProperty)
        {
            float value = getProperty(entities.First());

            foreach (GameEntity entity in entities.Skip(1))
            {
                if (!value.IsTheSameAs(getProperty(entity)))
                {
                    return null;
                }
            }

            return value;
        }

        /// <summary>
        /// Goes through a specified property in each selected entity
        /// and checks whether or not they are the same.
        /// </summary>
        /// <param name="entities">List of selected entities to check.</param>
        /// <param name="getProperty">Property in each selected entity to compare.</param>
        /// <returns>True, false, or null.</returns>
        public static bool? GetMixedValue(List<GameEntity> entities, Func<GameEntity, bool> getProperty)
        {
            bool value = getProperty(entities.First());

            foreach (GameEntity entity in entities.Skip(1))
            {
                if (value != getProperty(entity))
                {
                    return null;
                }
            }

            return value;
        }

        /// <summary>
        /// Goes through a specified property in each selected entity
        /// and checks whether or not they are the same.
        /// </summary>
        /// <param name="entities">List of selected entities to check.</param>
        /// <param name="getProperty">Property in each selected entity to compare.</param>
        /// <returns>True, false, or null.</returns>
        public static string GetMixedValue(List<GameEntity> entities, Func<GameEntity, string> getProperty)
        {
            string value = getProperty(entities.First());

            foreach (GameEntity entity in entities.Skip(1))
            {
                if (value != getProperty(entity))
                {
                    return null;
                }
            }

            return value;
        }

        // PROTECTED
        protected virtual bool UpdateGameEntities(string propertyName)
        {
            switch (propertyName)
            {
                case nameof(IsEnabled): 
                    SelectedEntities.ForEach(x => x.IsEnabled = IsEnabled.Value); 
                    return true;
                case nameof(Name):
                    SelectedEntities.ForEach(x => x.Name = Name);
                    return true;
            }
            return false;
        }
        
        protected virtual bool UpdateMSGameEntities()
        {
            IsEnabled = GetMixedValue(SelectedEntities, new Func<GameEntity, bool>(x => x.IsEnabled));
            Name = GetMixedValue(SelectedEntities, new Func<GameEntity, string>(x => x.Name));

            return true;
        }
    }

    class MSGameEntity : MSEntity
    {
        public MSGameEntity(List<GameEntity> entities) : base(entities)
        {
            Refresh();
        }
    }
}
