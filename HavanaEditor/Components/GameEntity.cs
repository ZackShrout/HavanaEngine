using HavanaEditor.DllWrapper;
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
    [KnownType(typeof(Script))]
    class GameEntity : ViewModelBase
    {
        // STATE
        private bool _isEnabled = true;
        private string _name;
        [DataMember(Name = nameof(Components))]
        private readonly ObservableCollection<Component> _components = new ObservableCollection<Component>();
        private int _entityID = ID.INVALID_ID;
        private bool _isActive;

        // PROPERTIES
        [DataMember]
        public bool IsEnabled
        {
            get => _isEnabled;
            set
            {
                if (_isEnabled != value)
                {
                    _isEnabled = value;
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }
        [DataMember]
        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }
        [DataMember]
        public Scene ParentScene { get; private set; }
        public ReadOnlyObservableCollection<Component> Components { get; private set; }
        public int EntityID
        {
            get => _entityID;
            set
            {
                if (_entityID != value)
                {
                    _entityID = value;
                    OnPropertyChanged(nameof(EntityID));
                }
            }
        }
        public bool IsActive
        {
            get => _isActive;
            set
            {
                if (_isActive != value)
                {
                    _isActive = value;
                    if (_isActive) // Add to engine if active
                    {
                        EntityID = EngineAPI.EntityAPI.CreateGameEntity(this);
                        Debug.Assert(ID.IsValid(_entityID));
                    }
                    else if (ID.IsValid(EntityID)) // Remove from engine if not
                    {
                        EngineAPI.EntityAPI.RemoveGameEntity(this);
                        EntityID = ID.INVALID_ID;
                    }

                    OnPropertyChanged(nameof(IsActive));
                }
            }
        }

        // PUBLIC
        public GameEntity(Scene scene)
        {
            Debug.Assert(scene != null);
            ParentScene = scene;
            _components.Add(new Transform(this));
            OnDeserialized(new StreamingContext());
        }

        /// <summary>
        /// Get specified component from it's parent entity. You must type-cast the value that comes back.
        /// </summary>
        /// <param name="type">- Type of component to get.</param>
        /// <returns>Component object.</returns>
        public Component GetComponent(Type type) => Components.FirstOrDefault(c => c.GetType() == type);

        /// <summary>
        /// Get specified component from it's parent entity.
        /// </summary>
        /// <typeparam name="T">- Type of component to get.</typeparam>
        /// <returns>Component object.</returns>
        public T GetComponent<T>() where T : Component => GetComponent(typeof(T)) as T;

        /// <summary>
        /// Add a component to an entity.
        /// </summary>
        /// <param name="component">The component to add.</param>
        /// <returns>True if successful, false if unsuccessful.</returns>
        public bool AddComponent(Component component)
        {
            Debug.Assert(component != null);
            if (!Components.Any(x => x.GetType() == component.GetType()))
            {
                IsActive = false;
                _components.Add(component);
                IsActive = true;
                return true;
            }
            Logger.Log(MessageType.Warning, $"Entity {Name} already has a {component.GetType().Name} component.");
            return false;
        }

        /// <summary>
        /// Remove a component from an entity.
        /// </summary>
        /// <param name="component">The component to remove.</param>
        public void RemoveComponent(Component component)
        {
            Debug.Assert(component != null);
            if (component is Transform) return; // Transform components cannot be removed

            if (_components.Contains(component))
            {
                IsActive = false;
                _components.Remove(component);
                IsActive = true;
            }
        }

        // PRIVATE
        [OnDeserialized]
        private void OnDeserialized(StreamingContext context)
        {
            if (_components != null)
            {
                Components = new ReadOnlyObservableCollection<Component>(_components);
                OnPropertyChanged(nameof(Components));
            }
        }
    }

    abstract class MSEntity : ViewModelBase
    {
        // STATE
        private bool _enableUpdates = true;
        private bool? _isEnabled = true;
        private string _name;
        private readonly ObservableCollection<IMSComponent> _components = new ObservableCollection<IMSComponent>();

        // PROPERTIES
        public bool? IsEnabled
        {
            get => _isEnabled;
            set
            {
                if (_isEnabled != value)
                {
                    _isEnabled = value;
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }
        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
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
            Components = new ReadOnlyObservableCollection<IMSComponent>(_components);
            SelectedEntities = entities;
            PropertyChanged += (s, e) => { if (_enableUpdates) UpdateGameEntities(e.PropertyName); };
        }

        /// <summary>
        /// Updates all entities in the selected entities list.
        /// </summary>
        public void Refresh()
        {
            _enableUpdates = false;
            UpdateMSGameEntities();
            MakeComponentList();
            _enableUpdates = true;
        }

        /// <summary>
        /// Goes through a specified property in each selected object
        /// and checks whether or not they are the same.
        /// </summary>
        /// <typeparam name="T">Type of object to look through.</typeparam>
        /// <param name="objects">List of selected entities to check.</param>
        /// <param name="getProperty">Property in each selected entity to compare.</param>
        /// <returns>True, false, or null.</returns>
        public static float? GetMixedValue<T>(List<T> objects, Func<T, float> getProperty)
        {
            float value = getProperty(objects.First());
            return objects.Skip(1).Any(x => !getProperty(x).IsTheSameAs(value)) ? (float?)null : value;
        }

        /// <summary>
        /// Goes through a specified property in each selected object
        /// and checks whether or not they are the same.
        /// </summary>
        /// <typeparam name="T">Type of object to look through.</typeparam>
        /// <param name="objects">List of selected entities to check.</param>
        /// <param name="getProperty">Property in each selected entity to compare.</param>
        /// <returns>True, false, or null.</returns>
        public static bool? GetMixedValue<T>(List<T> objects, Func<T, bool> getProperty)
        {
            bool value = getProperty(objects.First());
            return objects.Skip(1).Any(x => value != getProperty(x)) ? (bool?)null : value;
        }

        /// <summary>
        /// Goes through a specified property in each selected object
        /// and checks whether or not they are the same.
        /// </summary>
        /// <typeparam name="T">Type of object to look through.</typeparam>
        /// <param name="objects">List of selected entities to check.</param>
        /// <param name="getProperty">Property in each selected entity to compare.</param>
        /// <returns>True, false, or null.</returns>
        public static string GetMixedValue<T>(List<T> objects, Func<T, string> getProperty)
        {
            string value = getProperty(objects.First());
            return objects.Skip(1).Any(x => value != getProperty(x)) ? null : value;
        }

        /// <summary>
        /// Get specified multiselect component from it's parent entity.
        /// </summary>
        /// <typeparam name="T">- Type of multiselect component to get.</typeparam>
        /// <returns>Multiselect Component object.</returns>
        public T GetMSComponent<T>() where T : IMSComponent
        {
            return (T)Components.FirstOrDefault(x => x.GetType() == typeof(T));
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

        // PRIVATE
        private void MakeComponentList()
        {
            _components.Clear();
            GameEntity firstEntity = SelectedEntities.FirstOrDefault();
            if (firstEntity == null) return;

            foreach (Component component in firstEntity.Components)
            {
                Type type = component.GetType();
                if (!SelectedEntities.Skip(1).Any(entity => entity.GetComponent(type) == null))
                {
                    Debug.Assert(Components.FirstOrDefault(x => x.GetType() == type) == null);
                    _components.Add(component.GetMSComponent(this));
                }
            }
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
