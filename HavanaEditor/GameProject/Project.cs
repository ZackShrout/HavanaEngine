using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Runtime.Serialization;
using System.Text;

namespace HavanaEditor.GameProject
{
    /// <summary>
    /// Describes a Havana Project.
    /// </summary>
    [DataContract(Name = "Game")]
    public class Project : ViewModelBase
    {
        // STATE
        [DataMember(Name = "Scenes")]
        private ObservableCollection<Scene> scenes = new ObservableCollection<Scene>();

        // PROPERTIES
        public static string Extention { get; } = ".hvproj";
        [DataMember]
        public string Name { get; private set; }
        [DataMember]
        public string Path { get; private set; }
        public string FullPath => $"{Path}{Name}{Extention}";
        public ReadOnlyObservableCollection<Scene> Scenes { get; }

        // PUBLIC
        public Project(string name, string path)
        {
            Name = name;
            Path = path;

            scenes.Add(new Scene(this, "Default Scene"));
        }
    }
}
