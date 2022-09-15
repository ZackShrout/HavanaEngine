using HavanaEditor.GameProject;
using HavanaEditor.Utilities;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Threading;

namespace HavanaEditor.GameDev
{
    enum BuildConfiguration
    {
        Debug,
        DebugEditor,
        Release,
        ReleaseEditor
    }

    static class VisualStudio
    {
        // STATE
        private static readonly string _progID = "VisualStudio.DTE.17.0";
        private static readonly object _lock = new object();
        private static readonly string[] _buildConfigurationNames = new string[] { "Debug", "DebugEditor", "Release", "ReleaseEditor" };
        private static EnvDTE80.DTE2 _vsInstance = null;

        // PROPERTIES
        public static bool BuildSucceeded { get; private set; } = true;
        public static bool BuildDone { get; private set; } = true;

        // EVENTS
        private static readonly ManualResetEventSlim _resetEvent = new ManualResetEventSlim(false);

        // PUBLIC
        public static void OpenVisualStudio(string solutionPath)
        {
            lock (_lock) { OpenVisualStudio_Internal(solutionPath); }
        }

        public static void CloseVisualStudio()
        {
            lock (_lock) { CloseVisualStudio_Internal(); }
        }

        public static bool AddFilesToSolution(string solution, string projectName, string[] files)
        {
            lock (_lock) { return AddFilesToSolution_Internal(solution, projectName, files); }
        }

        public static void BuildSolution(Project project, BuildConfiguration buildConfig, bool showWindow = true)
        {
            lock (_lock) { BuildSolution_Internal(project, buildConfig, showWindow); }
        }

        public static bool IsDebugging()
        {
            lock (_lock) { return IsDebugging_Internal(); }
        }

        public static void Run(Project project, BuildConfiguration buildConfig, bool debug)
        {
            lock (_lock) { Run_Internal(project, buildConfig, debug); }
        }

        public static void Stop()
        {
            lock (_lock) { Stop_Internal(); }
        }

        public static string GetConfigurationName(BuildConfiguration config) => _buildConfigurationNames[(int)config];

        // PRIVATE
        /// <summary>
        /// Creates an instance of Visual Studio 2019, and opens solution
        /// at specified path.
        /// </summary>
        /// <param name="solutionPath">Path of the Visual Studio solution to open.</param>
        private static void OpenVisualStudio_Internal(string solutionPath)
        {
            IRunningObjectTable rot = null;
            IEnumMoniker monikerTable = null;
            IBindCtx bindCtx = null;

            try
            {
                if (_vsInstance == null)
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
                        if (name.Contains(_progID))
                        {
                            hResult = rot.GetObject(currentMoniker[0], out object obj);
                            if (hResult < 0 || obj == null) throw new COMException($"GetObject() returned HRESULT: {hResult:x8}");
                            EnvDTE80.DTE2 dte = obj as EnvDTE80.DTE2;
                            
                            string solutionName = string.Empty;
                            CallOnSTAThread(() =>
                            {
                                solutionName = dte.Solution.FullName;
                            });

                            if (solutionName == solutionPath)
                            {
                                _vsInstance = dte;
                                break;
                            }
                        }
                    }

                    // If we still can't find it, create a new instance of it
                    if (_vsInstance == null)
                    {
                        Type visualStudioType = Type.GetTypeFromProgID(_progID, true);
                        _vsInstance = Activator.CreateInstance(visualStudioType) as EnvDTE80.DTE2;
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
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
        private static void CloseVisualStudio_Internal()
        {
            CallOnSTAThread(() =>
            {
                if (_vsInstance?.Solution.IsOpen == true)
                {
                    _vsInstance.ExecuteCommand("File.SaveAll");
                    _vsInstance.Solution.Close(true);
                }
                _vsInstance?.Quit();
                _vsInstance = null;
            });
        }

        /// <summary>
        /// Add files to a Visual Studio solution
        /// </summary>
        /// <param name="solution">Path of solution to add files to.</param>
        /// <param name="projectName">Name of project in solutio to add files to.</param>
        /// <param name="files">Files to add.</param>
        /// <returns>True if succeeded, false if failed.</returns>
        private static bool AddFilesToSolution_Internal(string solution, string projectName, string[] files)
        {
            Debug.Assert(files?.Length > 0);
            OpenVisualStudio_Internal(solution);

            try
            {
                if (_vsInstance != null)
                {
                    CallOnSTAThread(() =>
                    {
                        if (!_vsInstance.Solution.IsOpen) _vsInstance.Solution.Open(solution);
                        else _vsInstance.ExecuteCommand("File.SaveAll");

                        foreach (EnvDTE.Project project in _vsInstance.Solution.Projects)
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
                            _vsInstance.ItemOperations.OpenFile(cpp, EnvDTE.Constants.vsViewKindTextView).Visible = true;
                        }

                        _vsInstance.MainWindow.Activate();
                        _vsInstance.MainWindow.Visible = true;
                    });
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
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
        private static void BuildSolution_Internal(Project project, BuildConfiguration buildConfig, bool showWindow = true)
        {
            if (IsDebugging_Internal())
            {
                Logger.Log(MessageType.Error, "Visual Studio is currently running a process.");
                return;
            }

            OpenVisualStudio_Internal(project.Solution);
            BuildDone = BuildSucceeded = false;

            CallOnSTAThread(() =>
            {
                _vsInstance.MainWindow.Visible = showWindow;
                if (!_vsInstance.Solution.IsOpen) _vsInstance.Solution.Open(project.Solution);
                _vsInstance.Events.BuildEvents.OnBuildProjConfigBegin += OnBuildSolutionBegin;
                _vsInstance.Events.BuildEvents.OnBuildProjConfigDone += OnBuildSolutionDone;
            });

            var configName = GetConfigurationName(buildConfig);

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

            CallOnSTAThread(() =>
            {
                _vsInstance.Solution.SolutionBuild.SolutionConfigurations.Item(buildConfig).Activate();
                _vsInstance.ExecuteCommand("Build.BuildSolution");
                _resetEvent.Wait();
                _resetEvent.Reset();
            });
        }

        private static bool IsDebugging_Internal()
        {
            bool result = false;
            CallOnSTAThread(() =>
            {
                result = _vsInstance != null &&
                    (_vsInstance.Debugger.CurrentProgram != null || _vsInstance.Debugger.CurrentMode == EnvDTE.dbgDebugMode.dbgRunMode);
            });
              
            return result;
        }

        private static void Run_Internal(Project project, BuildConfiguration buildConfig, bool debug)
        {
            CallOnSTAThread(() =>
            {
                if (_vsInstance != null && !IsDebugging_Internal() && BuildDone && BuildSucceeded)
                {
                    _vsInstance.ExecuteCommand(debug ? "Debug.Start" : "Debug.StartWithoutDebugging");
                }
            });
        }

        private static void Stop_Internal()
        {
            CallOnSTAThread(() =>
            {
                if (_vsInstance != null && IsDebugging_Internal())
                {
                    _vsInstance.ExecuteCommand("Debug.StopDebugging");
                }
            });
        }

        [DllImport("ole32.dll")]
        private static extern int GetRunningObjectTable(uint reserved, out IRunningObjectTable pprot);

        [DllImport("ole32.dll")]
        private static extern int CreateBindCtx(uint reserved, out IBindCtx ppbc);

        private static void CallOnSTAThread(Action action)
        {
            Debug.Assert(action != null);
            var thread = new Thread(() =>
            {
                MessageFilter.Register();
                try { action(); }
                catch (Exception ex) { Logger.Log(MessageType.Warning, ex.Message); }
                finally { MessageFilter.Revoke(); }
            });

            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();
            thread.Join();
        }

        private static void OnBuildSolutionBegin(string project, string projectConfig, string platform, string solutionConfig)
        {
            if (BuildDone) return;
            Logger.Log(MessageType.Info, $"Building: {project}, {projectConfig}, {platform}, {solutionConfig}");
        }

        private static void OnBuildSolutionDone(string project, string projectConfig, string platform, string solutionConfig, bool success)
        {
            if (BuildDone) return;

            if (success) Logger.Log(MessageType.Info, $"Building {projectConfig} configuration succeeded.");
            else Logger.Log(MessageType.Error, $"Building {projectConfig} configuration failed.");

            BuildDone = true;
            BuildSucceeded = success;
            _resetEvent.Set();
        }
    }

    // Class containing the IOleMessageFilter thread error-handling function
    public class MessageFilter : IMessageFilter
    {
        private const int SERVERCALL_ISHANDLED = 0;
        private const int PENDINGMSG_WAITDEFPROCESS = 2;
        private const int SERVERCALL_RETRYLATER = 2;

        [DllImport("Ole32.dll")]
        private static extern int CoRegisterMessageFilter(IMessageFilter newFilter, out IMessageFilter oldFilter);

        public static void Register()
        {
            IMessageFilter newFilter = new MessageFilter();
            int hr = CoRegisterMessageFilter(newFilter, out var oldFilter);
            Debug.Assert(hr >= 0, "Registring COM IMessageFilter failed.");
        }

        public static void Revoke()
        {
            int hr = CoRegisterMessageFilter(null, out var oldFilter);
            Debug.Assert(hr >= 0, "Unregistring COM IMessageFilter failed.");
        }

        int IMessageFilter.HandleInComingCall(int dwCallType, System.IntPtr hTaskCaller, int dwTickCount, System.IntPtr lpInterfaceInfo)
        {
            //returns the flag SERVERCALL_ISHANDLED. 
            return SERVERCALL_ISHANDLED;
        }

        int IMessageFilter.RetryRejectedCall(System.IntPtr hTaskCallee, int dwTickCount, int dwRejectType)
        {
            // Thread call was refused, try again. 
            if (dwRejectType == SERVERCALL_RETRYLATER)
            {
                // retry thread call at once, if return value >=0 & <=500.
                Debug.WriteLine("COM server busy. Retrying call to EnvDTE interface.");
                return 500;
            }
            // Too busy. Cancel call.
            return -1;
        }

        int IMessageFilter.MessagePending(System.IntPtr hTaskCallee, int dwTickCount, int dwPendingType)
        {
            return PENDINGMSG_WAITDEFPROCESS;
        }
    }

    [ComImport(), Guid("00000016-0000-0000-C000-000000000046"),
    InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    interface IMessageFilter
    {

        [PreserveSig]
        int HandleInComingCall(int dwCallType, IntPtr hTaskCaller, int dwTickCount, IntPtr lpInterfaceInfo);


        [PreserveSig]
        int RetryRejectedCall(IntPtr hTaskCallee, int dwTickCount, int dwRejectType);


        [PreserveSig]
        int MessagePending(IntPtr hTaskCallee, int dwTickCount, int dwPendingType);
    }
}
