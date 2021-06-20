using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Windows;

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
        private Scene activeScene;

        // PROPERTIES
        public static string Extention { get; } = ".hvproj";
        [DataMember]
        public string Name { get; private set; } = "New Project";
        [DataMember]
        public string Path { get; private set; }
        public string FullPath => $"{Path}{Name}{Extention}";
        public static Project Current => Application.Current.MainWindow.DataContext as Project;
        public ReadOnlyObservableCollection<Scene> Scenes { get; private set;  }
        public Scene ActiveScene
        {
            get => activeScene;
            set
            {
                if (activeScene != value)
                {
                    activeScene = value;
                    OnPropertyChanged(nameof(ActiveScene));
                }
            }
        }

        // PUBLIC
        public Project(string name, string path)
        {
            Name = name;
            Path = path;

            OnDeserialized(new StreamingContext());
        }

        /// <summary>
        /// Save specified project.
        /// </summary>
        /// <param name="project">Project to save.</param>
        public static void Save(Project project)
        {
            Serializer.ToFile<Project>(project, project.FullPath);
        }

        /// <summary>
        /// Load project from specified file path.
        /// </summary>
        /// <param name="file">File path to load project from.</param>
        /// <returns>Project object loaded.</returns>
        public static Project Load(string file)
        {
            Debug.Assert(File.Exists(file));
            return Serializer.FromFile<Project>(file);
        }

        public void Unload()
        {
            // TODO: implement
        }

        // PRIVATE
        [OnDeserialized]
        private void OnDeserialized(StreamingContext context)
        {
            if (scenes != null)
            {
                Scenes = new ReadOnlyObservableCollection<Scene>(scenes);
                OnPropertyChanged(nameof(Scenes));
            }
            ActiveScene = Scenes.FirstOrDefault(x => x.IsActive);
        }
    }
}
