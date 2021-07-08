using HavanaEditor.GameProject;
using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Text;

namespace HavanaEditor.GameDev
{
    static class VisualStudio
    {
        // STATE
        private static EnvDTE80.DTE2 vsInstance = null;
        private static readonly string progID = "VisualStudio.DTE.16.0";

        // PROPERTIES
        public static bool BuildSucceeded { get; private set; } = true;
        public static bool BuildDone { get; private set; } = true;

        // PUBLIC
        /// <summary>
        /// Creates an instance of Visual Studio 2019, and opens solution
        /// at specified path.
        /// </summary>
        /// <param name="solutionPath">Path of the Visual Studio solution to open.</param>
        public static void OpenVisualStudio(string solutionPath)
        {
            IRunningObjectTable rot = null;
            IEnumMoniker monikerTable = null;
            IBindCtx bindCtx = null;

            try
            {
                if (vsInstance == null)
                {
                    // Find an open instance of Visual Studio and check if it has the
                    // game solution already opened - if so, attach instance to it
                    int hResult = GetRunningObjectTable(0, out rot);
                    if (hResult < 0 || rot == null) throw new COMException($"GetRunningObjectTable() returned HRESULT: {hResult:x8}");

                    rot.EnumRunning(out monikerTable);
                    monikerTable.Reset();

                    hResult = CreateBindCtx(0, out bindCtx);
                    if (hResult < 0 || bindCtx == null) throw new COMException($"CreateBindCtx() returned HRESULT: {hResult:x8}");

                    IMoniker[] currentMoniker = new IMoniker[1];
                    while (monikerTable.Next(1, currentMoniker, IntPtr.Zero) == 0)
                    {
                        string name = string.Empty;
                        currentMoniker[0]?.GetDisplayName(bindCtx, null, out name);
                        if (name.Contains(progID))
                        {
                            hResult = rot.GetObject(currentMoniker[0], out object obj);
                            if (hResult < 0 || obj == null) throw new COMException($"GetObject() returned HRESULT: {hResult:x8}");

                            EnvDTE80.DTE2 dte = obj as EnvDTE80.DTE2;
                            string solutionName = dte.Solution.FullName;
                            if (solutionName == solutionPath)
                            {
                                vsInstance = dte;
                                break;
                            }
                        }
                    }


                    // If we still can't find it, create a new instance of it
                    if (vsInstance == null)
                    {
                        Type visualStudioType = Type.GetTypeFromProgID(progID, true);
                        vsInstance = Activator.CreateInstance(visualStudioType) as EnvDTE80.DTE2;
                    }
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);
                Logger.Log(MessageType.Error, "Failed to open Visual Studio.");
            }
            finally
            {
                if (rot != null) Marshal.ReleaseComObject(rot);
                if (monikerTable != null) Marshal.ReleaseComObject(monikerTable);
                if (bindCtx != null) Marshal.ReleaseComObject(bindCtx);
            }
        }

        /// <summary>
        /// Close the instance of Visual Studio that has the game solution opened,
        /// saving all files in the solution, as well as the solution itself, first.
        /// </summary>
        public static void CloseVisualStudio()
        {
            if (vsInstance?.Solution.IsOpen == true)
            {
                vsInstance.ExecuteCommand("File.SaveAll");
                vsInstance.Solution.Close(true);
            }
            vsInstance?.Quit();
        }

        /// <summary>
        /// Add files to a Visual Studio solution
        /// </summary>
        /// <param name="solution">Path of solution to add files to.</param>
        /// <param name="projectName">Name of project in solutio to add files to.</param>
        /// <param name="files">Files to add.</param>
        /// <returns>True if succeeded, false if failed.</returns>
        public static bool AddFilesToSolution(string solution, string projectName, string[] files)
        {
            Debug.Assert(files?.Length > 0);
            OpenVisualStudio(solution);

            try
            {
                if (vsInstance != null)
                {
                    if (!vsInstance.Solution.IsOpen) vsInstance.Solution.Open(solution);
                    else vsInstance.ExecuteCommand("File.SaveAll");

                    foreach (EnvDTE.Project project in vsInstance.Solution.Projects)
                    {
                        if (project.UniqueName.Contains(projectName))
                        {
                            foreach (string file in files)
                            {
                                project.ProjectItems.AddFromFile(file);
                            }
                        }
                    }

                    var cpp = files.FirstOrDefault(x => Path.GetExtension(x) == ".cpp");
                    if (!string.IsNullOrEmpty(cpp))
                    {
                        vsInstance.ItemOperations.OpenFile(cpp, EnvDTE.Constants.vsViewKindTextView).Visible = true;
                    }

                    vsInstance.MainWindow.Activate();
                    vsInstance.MainWindow.Visible = true;
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);
                Logger.Log(MessageType.Error, "Could not add files to solution.");
                return false;
            }
            
            return true;
        }

        /// <summary>
        /// Build Visual Studio solution
        /// </summary>
        /// <param name="project">Havana Project which contains the solution to build.</param>
        /// <param name="v">Build Configuration mode.</param>
        public static void BuildSolution(Project project, string buildConfig, bool showWindow = true)
        {
            if (IsDebugging())
            {
                Logger.Log(MessageType.Error, "Visual Studio is currently running a process.");
                return;
            }

            OpenVisualStudio(project.Solution);
            BuildDone = BuildSucceeded = false;

            for (int i = 0; i < 3 && !BuildDone; i++)
            {
                try
                {
                    if (!vsInstance.Solution.IsOpen) vsInstance.Solution.Open(project.Solution);
                    vsInstance.MainWindow.Visible = showWindow;

                    vsInstance.Events.BuildEvents.OnBuildProjConfigBegin += OnBuildSolutionBegin;
                    vsInstance.Events.BuildEvents.OnBuildProjConfigDone += OnBuildSolutionDone;

                    try
                    {
                        foreach (var pdbFile in Directory.GetFiles(Path.Combine($"{project.Path}", $@"x64\{buildConfig}"), "*.pdb"))
                        {
                            File.Delete(pdbFile);
                        }
                    }
                    catch (Exception ex)
                    {
                        Debug.WriteLine(ex.Message);
                    }

                    vsInstance.Solution.SolutionBuild.SolutionConfigurations.Item(buildConfig).Activate();
                    vsInstance.ExecuteCommand("Build.BuildSolution");
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                    Debug.WriteLine($"Attempt {i}: failed to buil {project.Name}.");
                    System.Threading.Thread.Sleep(1000);
                }
            }
        }

        public static bool IsDebugging()
        {
            bool result = false;
            bool tryAgain = true;

            for (int i = 0; i < 3 && tryAgain; i++)
            {
                try
                {
                    result = vsInstance != null &&
                        (vsInstance.Debugger.CurrentProgram != null || vsInstance.Debugger.CurrentMode == EnvDTE.dbgDebugMode.dbgRunMode);
                    tryAgain = false;
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                    System.Threading.Thread.Sleep(1000);
                }
            }
            return result;
        }

        public static void Run(Project project, string configName, bool debug)
        {
            if (vsInstance != null && !IsDebugging() && BuildDone && BuildSucceeded)
            {
                vsInstance.ExecuteCommand(debug ? "Debug.Start" : "Debug.StartWithoutDebugging");
            }
        }

        public static void Stop()
        {
            if (vsInstance != null && IsDebugging())
            {
                vsInstance.ExecuteCommand("Debug.StopDebugging");
            }
        }

        // PRIVATE
        [DllImport("ole32.dll")]
        private static extern int GetRunningObjectTable(uint reserved, out IRunningObjectTable pprot);

        [DllImport("ole32.dll")]
        private static extern int CreateBindCtx(uint reserved, out IBindCtx ppbc);
        
        private static void OnBuildSolutionBegin(string project, string projectConfig, string platform, string solutionConfig)
        {
            Logger.Log(MessageType.Info, $"Building: {project}, {projectConfig}, {platform}, {solutionConfig}");
        }

        private static void OnBuildSolutionDone(string project, string projectConfig, string platform, string solutionConfig, bool success)
        {
            if (BuildDone) return;

            if (success) Logger.Log(MessageType.Info, $"Building {projectConfig} configuration succeeded.");
            else Logger.Log(MessageType.Error, $"Building {projectConfig} configuration failed.");

            BuildDone = true;
            BuildSucceeded = success;
        }
    }
}
