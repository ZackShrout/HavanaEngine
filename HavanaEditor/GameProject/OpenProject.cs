using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;

namespace HavanaEditor.GameProject
{
    /// <summary>
    /// Describes specific data for each project created in Havana Editor...
    /// Name, Path, and Date.
    /// </summary>
    [DataContract]
    public class ProjectData
    {
        // PROPERTIES
        [DataMember]
        public string ProjectName { get; set; }
        [DataMember]
        public string ProjectPath { get; set; }
        [DataMember]
        public DateTime Date { get; set; }
        public string FullPath { get => $"{ProjectPath}{ProjectName}{Project.Extention}"; }
        public byte[] Icon { get; set; }
        public byte[] ScreenShot { get; set; }

    }

    /// <summary>
    /// List of all the pojects that have been created with Havana Editor, as ProjectData.
    /// </summary>
    [DataContract]
    public class ProjectDataList
    {
        // PROPERTIES
        [DataMember]
        public List<ProjectData> Projects { get; set; }
    }

    /// <summary>
    /// View Model that handles the opening of a Havana Project, and inherits from
    /// HavanaEditor.ViewModelBase
    /// </summary>
    class OpenProject
    {
        // STATE
        private static readonly string _applicationDataPath = $@"{Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)}\HavanaEditor\";
        private static readonly string _projectDataPath;
        private static readonly ObservableCollection<ProjectData> _projects = new ObservableCollection<ProjectData>();

        // PROPERTIES
        public static ReadOnlyObservableCollection<ProjectData> Projects { get; }

        // PUBLIC
        static OpenProject()
        {
            try
            {
                if(!Directory.Exists(_applicationDataPath))
                {
                    Directory.CreateDirectory(_applicationDataPath);
                }
                _projectDataPath = $@"{_applicationDataPath}ProjectData.xml";
                Projects = new ReadOnlyObservableCollection<ProjectData>(_projects);
                ReadProjectData();
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Error, $"Failed to open read project data.");
                throw;
            }
        }

        /// <summary>
        /// Open a specified project in HavanaEditor.
        /// </summary>
        /// <param name="data">The project to open.</param>
        /// <returns>Project object.</returns>
        public static Project Open(ProjectData data)
        {
            ReadProjectData();
            ProjectData project = Projects.FirstOrDefault(x => x.FullPath == data.FullPath);
            
            // If project is in the list already, update the date
            if (project != null) 
            {
                project.Date = DateTime.Now;
            }
            else // If not, add it and update the date
            {
                project = data;
                project.Date = DateTime.Now;
                _projects.Add(project);
            }
            WriteProjectData();

            return Project.Load(project.FullPath);
        }

        // PRIVATE
        private static void ReadProjectData()
        {
            if (File.Exists(_projectDataPath))
            {
                var newProjects = Serializer.FromFile<ProjectDataList>(_projectDataPath).Projects.OrderByDescending(x => x.Date);
                _projects.Clear();
                foreach (var project in newProjects)
                {
                    if (File.Exists(project.FullPath))
                    {
                        project.Icon = File.ReadAllBytes($@"{project.ProjectPath}\.havana\Icon.png");
                        project.ScreenShot = File.ReadAllBytes($@"{project.ProjectPath}\.havana\ScreenShot.png");
                        _projects.Add(project);
                    }
                }
            }
        }

        private static void WriteProjectData()
        {
            List<ProjectData> newProjects = _projects.OrderBy(x => x.Date).ToList();
            Serializer.ToFile(new ProjectDataList() { Projects = newProjects }, _projectDataPath);
        }
    }
}
