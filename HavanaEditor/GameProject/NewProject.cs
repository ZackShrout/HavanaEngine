using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;

namespace HavanaEditor.GameProject
{
    /// <summary>
    /// Class that describes the contents of the different templates available
    /// in the Havana Editor.
    /// </summary>
    [DataContract]
    public class ProjectTemplate
    {
        [DataMember]
        public string ProjectType { get; set; }
        [DataMember]
        public string ProjectFile { get; set; }
        [DataMember]
        public List<string> Folders { get; set; }
        public byte[] Icon { get; set; }
        public byte[] ScreenShot { get; set; }
        public string IconFilePath { get; set; }
        public string ScreenShotFilePath { get; set; }
        public string ProjectFilePath { get; set; }
    }
    
    /// <summary>
    /// View Model that handles the creation of a new Havana Project, and inherits from
    /// HavanaEditor.ViewModelBase
    /// </summary>
    class NewProject : ViewModelBase
    {
        // STATE
        private readonly string templatePath = @"..\..\HavanaEditor\ProjectTemplates"; // TODO: get path from install location
        private string projectName = "NewProject";
        private string projectPath = $@"{Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments)}\HavanaProjects\";
        private ObservableCollection<ProjectTemplate> projectTemplates = new ObservableCollection<ProjectTemplate>();

        // PROPERTIES
        public string ProjectName
        {
            get => projectName;
            set
            {
                if (projectName != value)
                {
                    projectName = value;
                    OnPropertyChanged(nameof(ProjectName));
                }
            }
        }
        
        public string ProjectPath
        {
            get => projectPath;
            set
            {
                if (projectPath != value)
                {
                    projectPath = value;
                    OnPropertyChanged(nameof(ProjectPath));
                }
            }
        }
        
        public ReadOnlyObservableCollection<ProjectTemplate> ProjectTemplates { get; }
        
        // PUBLIC
        public NewProject()
        {
            ProjectTemplates = new ReadOnlyObservableCollection<ProjectTemplate>(projectTemplates);
            try
            {
                string[] templateFiles = Directory.GetFiles(templatePath, "template.xml", SearchOption.AllDirectories);
                Debug.Assert(templateFiles.Any());
                foreach (var file in templateFiles)
                {
                    ProjectTemplate template = Serializer.FromFile<ProjectTemplate>(file);
                    template.IconFilePath = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(file), "Icon.png"));
                    template.Icon = File.ReadAllBytes(template.IconFilePath);
                    template.ScreenShotFilePath = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(file), "ScreenShot.png"));
                    template.ScreenShot = File.ReadAllBytes(template.ScreenShotFilePath);
                    template.ProjectFilePath = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(file), template.ProjectFile));
                    projectTemplates.Add(template);
                }
            }
            catch(Exception e)
            {
                Debug.WriteLine(e.Message);
                // TODO: log error
            }

        }
    }
}
