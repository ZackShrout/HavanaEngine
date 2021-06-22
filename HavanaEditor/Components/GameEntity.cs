﻿using HavanaEditor.GameProject;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Text;

namespace HavanaEditor.Components
{
    [DataContract]
    [KnownType(typeof(Transform))]
    public class GameEntity : ViewModelBase
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
}
