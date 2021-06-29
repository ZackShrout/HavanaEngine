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
    /// Model that describes the contents of the different templates available
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
        public string TemplatePath { get; set; }
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
        private bool isValid;
        private string errorMessage;

        // PROPERTIES
        public string ProjectName
        {
            get => projectName;
            set
            {
                if (projectName != value)
                {
                    projectName = value;
                    ValidateProjectPath();
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
                    ValidateProjectPath();
                    OnPropertyChanged(nameof(ProjectPath));
                }
            }
        }
        public ReadOnlyObservableCollection<ProjectTemplate> ProjectTemplates { get; }
        public bool IsValid
        {
            get => isValid;
            set
            {
                if (isValid != value)
                {
                    isValid = value;
                    OnPropertyChanged(nameof(IsValid));
                }
            }
        }
        public string ErrorMessage
        {
            get => errorMessage;
            set
            {
                if (errorMessage != value)
                {
                    errorMessage = value;
                    OnPropertyChanged(nameof(ErrorMessage));
                }
            }
        }

        // PUBLIC
        public NewProject()
        {
            ProjectTemplates = new ReadOnlyObservableCollection<ProjectTemplate>(projectTemplates);
            try
            {
                string[] templateFiles = Directory.GetFiles(templatePath, "template.xml", SearchOption.AllDirectories);
                Debug.Assert(templateFiles.Any());
                foreach (string file in templateFiles)
                {
                    ProjectTemplate template = Serializer.FromFile<ProjectTemplate>(file);
                    template.IconFilePath = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(file), "Icon.png"));
                    template.Icon = File.ReadAllBytes(template.IconFilePath);
                    template.ScreenShotFilePath = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(file), "ScreenShot.png"));
                    template.ScreenShot = File.ReadAllBytes(template.ScreenShotFilePath);
                    template.ProjectFilePath = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(file), template.ProjectFile));
                    template.TemplatePath = Path.GetDirectoryName(file);
                    projectTemplates.Add(template);
                }
                ValidateProjectPath();
            }
            catch(Exception e)
            {
                Debug.WriteLine(e.Message);
                Logger.Log(MessageTypes.Error, "Failed to read project templates.");
                throw;
            }

        }

        /// <summary>
        /// Creates a new project file and directory system for a specified template.
        /// </summary>
        /// <param name="template">Project template to create.</param>
        /// <returns>Path of project directory.</returns>
        public string CreateProject(ProjectTemplate template)
        {   
            if (!ValidateProjectPath())
            {
                return string.Empty;
            }

            if (!Path.EndsInDirectorySeparator(ProjectPath))
            {
                ProjectPath += @"\";
            }

            string path = $@"{ProjectPath}{ProjectName}\";
            try
            {
                if (!Directory.Exists(path))
                {
                    Directory.CreateDirectory(path);
                }
                foreach (string folder in template.Folders)
                {
                    Directory.CreateDirectory(Path.GetFullPath(Path.Combine(Path.GetDirectoryName(path), folder)));
                }
                DirectoryInfo directoryInfo = new DirectoryInfo(path + @".Havana\");
                directoryInfo.Attributes |= FileAttributes.Hidden;
                File.Copy(template.IconFilePath, Path.GetFullPath(Path.Combine(directoryInfo.FullName, "Icon.png")));
                File.Copy(template.ScreenShotFilePath, Path.GetFullPath(Path.Combine(directoryInfo.FullName, "ScreenShot.png")));

                string projectXML = File.ReadAllText(template.ProjectFilePath);
                projectXML = string.Format(projectXML, ProjectName, ProjectPath);
                string projectPath = Path.GetFullPath(Path.Combine(path, $"{ProjectName}{Project.Extention}"));
                File.WriteAllText(projectPath, projectXML);

                CreateMSVCSolution(template, path);

                return path;
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);
                Logger.Log(MessageTypes.Error, $"Failed to create {ProjectName}");
                throw;
            }
        }

        // PRIVATE
        private void CreateMSVCSolution(ProjectTemplate template, string projectPath)
        {
            Debug.Assert(File.Exists(Path.Combine(template.TemplatePath, "MSVCSolution")));
            Debug.Assert(File.Exists(Path.Combine(template.TemplatePath, "MSVCProject")));

            string engineAPIPath = Path.Combine(MainWindow.HavanaPath, @"Engine\EngineAPI\");
            Debug.Assert(Directory.Exists(engineAPIPath));

            string projectName = ProjectName;
            string projectGUID = "{" + Guid.NewGuid().ToString().ToUpper() + "}";

            // Create the Visual Studio solution file
            string solution = File.ReadAllText(Path.Combine(template.TemplatePath, "MSVCSolution"));
            solution = string.Format(solution, projectName, projectGUID, "{" + Guid.NewGuid().ToString().ToUpper() + "}");
            File.WriteAllText(Path.GetFullPath(Path.Combine(projectPath, $"{projectName}.sln")), solution);

            // Create the Visual Studio project file
            string project = File.ReadAllText(Path.Combine(template.TemplatePath, "MSVCProject"));
            project = string.Format(project, projectName, projectGUID, engineAPIPath, MainWindow.HavanaPath);
            File.WriteAllText(Path.GetFullPath(Path.Combine(projectPath, $@"Scripts\{projectName}.vcxproj")), project);
        }

        private bool ValidateProjectPath()
        {
            string path = ProjectPath;
            if (!Path.EndsInDirectorySeparator(path))
            {
                path += @"\";
            }
            path += $@"{ProjectName}\";

            IsValid = false;
            if (string.IsNullOrWhiteSpace(ProjectName.Trim()))
            {
                ErrorMessage = "Please type in a project name.";
            }
            else if (ProjectName.IndexOfAny(Path.GetInvalidFileNameChars()) != -1)
            {
                ErrorMessage = "Invalid character(s) in project name.";
            }
            else if (string.IsNullOrWhiteSpace(ProjectPath.Trim()))
            {
                ErrorMessage = "Please select a project path.";
            }
            else if (ProjectPath.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                ErrorMessage = "Invalid character(s) in project path.";
            }
            else if (Directory.Exists(path) && Directory.EnumerateFileSystemEntries(path).Any())
            {
                ErrorMessage = "Selected project path is already in use.";
            }
            else
            {
                ErrorMessage = string.Empty;
                IsValid = true;
            }

            return IsValid;
        }
    }
}
