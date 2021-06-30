﻿using HavanaEditor.DllWrapper;
using HavanaEditor.GameDev;
using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace HavanaEditor.GameProject
{
    enum BuildConfiguration
    {
        Debug,
        DebugEditor,
        Release,
        ReleaseEditor
    }
    
    /// <summary>
    /// Describes a Havana Project.
    /// </summary>
    [DataContract(Name = "Game")]
    class Project : ViewModelBase
    {
        // STATE
        [DataMember(Name = "Scenes")]
        private ObservableCollection<Scene> scenes = new ObservableCollection<Scene>();
        private Scene activeScene;
        private static readonly string[] buildConfigurationNames = new string[] { "Debug", "DebugEditor", "Release", "ReleaseEditor" };
        private int buildConfig;
        
        // PROPERTIES
        public static string Extention { get; } = ".hvproj";
        [DataMember]
        public string Name { get; private set; } = "New Project";
        [DataMember]
        public string Path { get; private set; }
        public string FullPath => $@"{Path}{Name}{Extention}";
        public string Solution => $@"{Path}{Name}.sln";
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
        [DataMember]
        public int BuildConfig
        {
            get => buildConfig;
            set
            {
                if (buildConfig != value)
                {
                    buildConfig = value;
                    OnPropertyChanged(nameof(BuildConfig));
                }
            }
        }
        public ICommand UndoCommand { get; private set; }
        public ICommand RedoCommand { get; private set; }
        public ICommand AddSceneCommand { get; private set; }
        public ICommand RemoveSceneCommand { get; private set; }
        public ICommand SaveCommand { get; private set; }
        public ICommand BuildCommand { get; private set; }
        public static UndoRedo UndoRedo { get; } = new UndoRedo();
        public BuildConfiguration StandAloneBuildConfig => BuildConfig == 0 ? BuildConfiguration.Debug : BuildConfiguration.Release;
        public BuildConfiguration DllBuildConfig => BuildConfig == 0 ? BuildConfiguration.DebugEditor : BuildConfiguration.ReleaseEditor;
        
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
            Logger.Log(MessageTypes.Info, $"Project saved to {project.FullPath}");
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

        /// <summary>
        /// Unload the Havana project file.
        /// </summary>
        public void Unload()
        {
            VisualStudio.CloseVisualStudio();
            UndoRedo.Reset();
        }

        // PRIVATE
        [OnDeserialized]
        private async void OnDeserialized(StreamingContext context)
        {
            if (scenes != null)
            {
                Scenes = new ReadOnlyObservableCollection<Scene>(scenes);
                OnPropertyChanged(nameof(Scenes));
            }
            ActiveScene = Scenes.FirstOrDefault(x => x.IsActive);

            await BuildGameCodeDll(false); // build dll without showing the Visual Studio window.

            SetCommands();
        }

        private async Task BuildGameCodeDll(bool showWindow = true)
        {
            try
            {
                UnloadGameCodeDll();
                await Task.Run(() => VisualStudio.BuildSolution(this, GetConfigurationName(DllBuildConfig), showWindow));
                if (VisualStudio.BuildSucceeded)
                {
                    LoadGameCodeDll();
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageTypes.Error, "Failed to build game.");
                throw;
            }

        }

        private void LoadGameCodeDll()
        {
            string configName = GetConfigurationName(DllBuildConfig);
            string dll = $@"{Path}x64\{configName}\{Name}.dll";
            if (File.Exists(dll) && EngineAPI.LoadGameCodeDll(dll) != 0)
            {
                Logger.Log(MessageTypes.Info, "Game code DLL loaded sucessfully.");
            }
            else
            {
                Logger.Log(MessageTypes.Warning, "Game code DLL did not load sucessfully. Did you build the project?");
            }

        }

        private void UnloadGameCodeDll()
        {
            if (EngineAPI.UnloadGameCodeDll() != 0)
            {
                Logger.Log(MessageTypes.Info, "Game code DLL unloaded sucessfully.");
            }
        }

        private void AddScene(string sceneName)
        {
            Debug.Assert(!string.IsNullOrEmpty(sceneName.Trim()));
            scenes.Add(new Scene(this, sceneName));
        }

        private void RemoveScene(Scene scene)
        {
            Debug.Assert(scenes.Contains(scene));
            scenes.Remove(scene);
        }

        private static string GetConfigurationName(BuildConfiguration config) => buildConfigurationNames[(int)config];

        private void SetCommands()
        {
            AddSceneCommand = new RelayCommand<object>(x =>
            {
                AddScene($"New Scene {scenes.Count()}");
                Scene newScene = scenes.Last();
                int sceneIndex = scenes.Count - 1;
                UndoRedo.Add(new UndoRedoAction(
                    () => RemoveScene(newScene),
                    () => scenes.Insert(sceneIndex, newScene),
                    $"Add {newScene.Name}"));
            });

            RemoveSceneCommand = new RelayCommand<Scene>(x => {
                int sceneIndex = scenes.IndexOf(x);
                RemoveScene(x);
                UndoRedo.Add(new UndoRedoAction(
                    () => scenes.Insert(sceneIndex, x),
                    () => RemoveScene(x),
                    $"Remove {x.Name}"));
            }, x => !x.IsActive);

            UndoCommand = new RelayCommand<object>(x => UndoRedo.Undo(), x => UndoRedo.UndoList.Any());
            RedoCommand = new RelayCommand<object>(x => UndoRedo.Redo(), x => UndoRedo.RedoList.Any());
            SaveCommand = new RelayCommand<object>(x => Save(this));
            BuildCommand = new RelayCommand<bool>(async x => await BuildGameCodeDll(), x => !VisualStudio.IsDebugging() && VisualStudio.BuildDone);

            OnPropertyChanged(nameof(AddSceneCommand));
            OnPropertyChanged(nameof(RemoveSceneCommand));
            OnPropertyChanged(nameof(UndoCommand));
            OnPropertyChanged(nameof(RedoCommand));
            OnPropertyChanged(nameof(SaveCommand));
            OnPropertyChanged(nameof(BuildCommand));
        }
    }
}
