using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Text;

namespace HavanaEditor.GameProject
{
    /// <summary>
    /// Describes any scene that may exist within a Havana Project.
    /// </summary>
    [DataContract]
    public class Scene : ViewModelBase
    {
        // STATE
        private string name;
        private bool isActive;
        
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

        // PUBLIC
        public Scene(Project project, string name)
        {
            Debug.Assert(project != null);
            Project = project;
            Name = name;
        }
    }
}
